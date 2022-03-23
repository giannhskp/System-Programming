#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <libgen.h>
#include <fcntl.h>
#include <poll.h>
#include "bloomFilter/bloomFilter.h"
#include "readDirs.h"
#include "database/database.h"
#include "linkedLists/bloomList.h"
#include "pipeReadWrite.h"
#include "travelMonitorFunctions.h"
#include "linkedLists/requestList.h"
#include "errorChecking/errorChecking.h"

# define READ 0
# define WRITE 1
int buffSize;   // the size of the buffer (given by the user)
// every time we send/receive a message through a pipe we write/read at most buffSize bytes (see pipeReadWrite.c file)
int bloomSize;   // the size of every bloom filter (given by the user)
int numMonitors;  // the number of monitors that will be created (given by the users)

int terminated=0; // flag that shows if a SIGINT or SIGQUIT has been received
int childTerminated=0;  // flag that shows if a SIGCHLD has been received


void SigchldHandler();
void IntQuitHandler();
void restoreChildren(int (*)[], int (*)[][2], stringList *,bloomList (*)[], char []);
void writeLogFile(stringList ,int ,int ,int ,stringList *);
void userRequest(char [], int *,int *,int *,requestList *,int [], int [][2], bloomList (*)[],stringList []);
void killChildren(int []);

int main(int argc, char* argv[]){
  if(argc<8){
    printf("Wrong input\nUsage: ./travelMonitor –m numMonitors -b bufferSize -s sizeOfBloom-i input_dir\n");
    return 0;
  }
  char *input_dir; // the path of the input directory given by the user

  int count=1;
  int read1=0,read2=0,read3=0,read4=0;  // for wrong input check
  while(count<=7){  //read  the arguments of the command line
    // arguments can be read in any sequence
    if(strcmp("-i",argv[count])==0){  // -i argument followed by the input_dir name
      input_dir=malloc(sizeof(char)*(strlen(argv[count+1])+1));
      strcpy(input_dir,argv[count+1]);
      read1=1;
      count+=2;
      continue;
    }
    if(strcmp("-b",argv[count])==0){  // -b for buffSize
      buffSize=atoi(argv[count+1]);
      read2=1;
      count+=2;
      continue;
    }
    if(strcmp("-m",argv[count])==0){  // -m for numMonitors
      numMonitors=atoi(argv[count+1]);
      read3=1;
      count+=2;
      continue;
    }
    if(strcmp("-s",argv[count])==0){  // -s for sizeOfBloom (bloomSize)
      bloomSize=atoi(argv[count+1]);
      read4=1;
      count+=2;
      continue;
    }
    count+=2;
  }
  if( !read1 || !read2 || !read3 || !read4){    // check if all arguments were read
    printf("Wrong input\n");
    return 0;
  }


  int totalTravelReqests=0; // stores the total number of request that the travelMonitor received by the user
  int acceptedRequests=0; // stores how many of the requests were accepted
  int rejectedRequests=0; // stores how many of the requests were rejected
  int fdboard[numMonitors][2];  // a 2D array that stores the file descriptors of all the pipes that the travelMonitor uses in order to comunicate with the monitors
  // we uses 2 pipes for every monitor. One for travelMonitor write - monitor read and one for travelMonitor read - monitor write
  bloomList bloomFiltersOfMonitors[numMonitors];  // array of lists that contains all the bloomFilters of a monitor
  stringList countriesOfMonitors[numMonitors]; // array of lists that contains all the countries that a monitor is "responsible" for
  int pidArray[numMonitors];  // an array that stores the process id of every monitor
  requestList listOfRequests=initializeRequestList(); // a list that stores info for every request received by the user

  struct sigaction signalAction;  // handling the signals that we receive using sigaction
  signalAction.sa_handler = SigchldHandler;  // set our function as the handler (implemented at the bottom of the file)
  sigemptyset(&signalAction.sa_mask);   //initialize
  signalAction.sa_flags = SA_RESTART;
  sigaction(SIGCHLD, &signalAction, NULL);  // handle the SIGCHLD signals
  //
  signalAction.sa_handler = IntQuitHandler;  // set our function as the handler (implemented at the bottom of the file)
  sigemptyset(&signalAction.sa_mask);   //initialize
  signalAction.sa_flags = SA_RESTART;
  sigaction(SIGINT, &signalAction, NULL); // handle the SIGINT signals
  sigaction(SIGQUIT, &signalAction, NULL);  // handle the SIGQUIT signals

  for(int i=0;i<numMonitors;i++){  // loop that creates the childs (monitors), in every loop a child is created
    char fifoNameRead[100];
    sprintf(fifoNameRead,"CHILD%dREAD",i);
    //create the name of the pipe that the father/travelMonitor will be writing and the monitor will be reading
    if(mkfifo(fifoNameRead,0777) == -1){ if(errno!=EEXIST) return -1; }

    char fifoNameWrite[100];
    sprintf(fifoNameWrite,"CHILD%dWRITE",i);
    //create the name of the pipe that the father/travelMonitor will be reading and the monitor will be writing
    if(mkfifo(fifoNameWrite,0777) == -1){ if(errno!=EEXIST) return -1; }

    int pid=fork();
    if(pid==-1){ perror("fork: "); exit(-1); }

  	if (pid==0){ //child process
      char *buffer[]={"./Monitor",NULL};    //convert the name of the executable into a string so as to be given as an argument to execl
      int err=execl(buffer[0],buffer[0],fifoNameRead,fifoNameWrite,(char *)NULL);  // call the monitor executable with the 2 pipes as parameters
      printf("execl error with exit code: %d\n",err); // if execl failed
      perror("execl");
  	}else{ // father process
      // open the reading pipe and store the file descriptor in to the array.
      if( (fdboard[i][READ] = open(fifoNameRead,O_RDONLY)) < 0){ perror("open: "); return -1; }
      // open the writing pipe and store the file descriptor in to the array.
      if( (fdboard[i][WRITE] = open(fifoNameWrite,O_WRONLY)) < 0){ perror("open: "); return -1; }
      // initialize the data stored for this monitor
      bloomFiltersOfMonitors[i]=initializeBloomList();  // initialize the list of bloomFilters of this child
      countriesOfMonitors[i]=initializeStringList();  // initialize the list of countries of this child
      pidArray[i]=pid;  // store the process id of this children
    }
  }

  for(int i=0;i<numMonitors;i++){ // for every monitor
    // send them the buffSize and the bloom filter size
    if( write(fdboard[i][WRITE],&buffSize,4) == -1 ){ perror("write: "); exit(-1); }  // send the buffSize
    sendInt(fdboard[i][WRITE],bloomSize); // send the bloomSize
  }
  stringList countries=readDirs(input_dir); // read the input directory and get the path of every directory
  // country list is sorted by country name
  stringList templist=countries;
  int roundRobinCounter=0;
  int countryCounter=0; // counts the number of countries/directories
  while(templist!=NULL){  // for every country/directory
    // send to the corresponding monitor the path of the directory (Round Robin is used to assing the countries to the monitors)
    sendMessage(fdboard[roundRobinCounter][WRITE],templist->string);
    char *tempBuff=strdup(templist->string);
    char *directoryName=basename(tempBuff); // get the name of the country (= the name of the directory)
    // and insert the country to the country list that corresponds to this monitor
    countriesOfMonitors[roundRobinCounter] = insertString(countriesOfMonitors[roundRobinCounter],directoryName);
    free(tempBuff);
    roundRobinCounter = (roundRobinCounter+1)%numMonitors;
    countryCounter++;
    templist=templist->next;
  }
  int oldNumOfMonitors=numMonitors;
  if(countryCounter<numMonitors)  // if the number of countries is less than the number of monitors
    numMonitors=countryCounter; // some of the monitors are not used, so only keep as many monitors as the number of countries
  deleteStringList(countries);
  char buff[]="ENDOFCOUNTRIES";
  // after assinging all the counties to the monitors, send a message to every monitor in order to show that the country assignment has finished
  for(int i=0;i<oldNumOfMonitors;i++){
    sendMessage(fdboard[i][WRITE],buff);
  }

  int ended[numMonitors]; // flag array that contains if the reading from every pipe if finished
  struct pollfd pfds[numMonitors];  // use poll to monitor all the pipes
  // each place of the poll board corresponds to the pipe of the equivelent children
  for(int i=0;i<numMonitors;i++){
    ended[i]=0;   // initialize the end board with zero as we have not started reading yet
    pfds[i].fd=fdboard[i][READ];  // add the pipe descriptor of the equivelent pipe to the poll array
    pfds[i].events=POLLIN;      // we are only intersted in reading so we add only POLLIN in the inrested events of poll
  }
  int flag=1;
  // start receiving/reading the bloom filters of every monitor
  while(flag){
    int retval = poll(pfds,numMonitors,0);
    flag=1;
    if(retval<0){ //if poll is interrupted by a signal, continue and redo poll
        flag=1;
        continue;
    }
    for(int i=0;i<numMonitors;i++){ // for every monitor, check if it has written to the pipe
      if((pfds[i].revents&POLLIN)){ //if something is written in this pipe
          char *virusName=readMessage(fdboard[i][READ]);  //read the message
          if(strcmp(virusName,"ENDOFBLOOMFILTERS")==0){ // that means that the monitor has sent all the bloomFilters
            free(virusName);
            ended[i]=1; // the monitor has ended sending the bloomFilters
          }else{
            bloomFilter bf = readBloom(fdboard[i][READ]); // read the bloom filter that corresponds to the virusName
            // add the bloom filter to the bloom filter list of this monitor
            bloomFiltersOfMonitors[i] = insertBloomToList(bloomFiltersOfMonitors[i],virusName,bf);
          }
      }
    }
    flag=0;
   //check if we have finished reading from all the pipes
    for(int i=0;i<numMonitors;i++){   //check if all monitors have ended sending the bloom filters
      if(ended[i]==0)
        flag=1;   // if we have not finished reading from at least one pipe, loop until one pipe is ready for reading
      // if we have finished reading from all the pipes the flag is 0 so the loop (and the reading) is stopped
    }
  }

  while(1){ // start receiving requests from the user
    char readBuff[500];
    printf(">: ");
    if(fgets(readBuff,500,stdin)==NULL){   //get command that the user gave
      perror("fgets: ");
      return -1;
    }
    if(strcmp(readBuff,"/exit\n")==0){  // if user gave /exit command
      terminated=1; // stop the loop
    }
    if(childTerminated==1)
      restoreChildren(&pidArray,&fdboard,countriesOfMonitors,&bloomFiltersOfMonitors,input_dir);
    if(terminated==1)
      break;
    userRequest(readBuff,&totalTravelReqests,&acceptedRequests,&rejectedRequests,&listOfRequests,pidArray,fdboard,&bloomFiltersOfMonitors,countriesOfMonitors);  // execute the request/command of the user
  }
  // if /exit command was received from user, or a SIGINT/SIGQUIT signal was received
  killChildren(pidArray); // send SIGKILL signal to all the children/monitors
  writeLogFile(countries,totalTravelReqests,acceptedRequests,rejectedRequests,countriesOfMonitors);  // write to the logFile
  int wpid,status=0;
  while (( wpid = wait(&status)) > 0);  // wait for all the childs to finish executing

  // free up all the memory that was allocated and close the open pipes
  deleteRequestList(listOfRequests);
  for(int i=0;i<numMonitors;i++){
    deleteBloomList(bloomFiltersOfMonitors[i]);
    deleteStringList(countriesOfMonitors[i]);
    close(fdboard[i][READ]);
    close(fdboard[i][WRITE]);
  }
  free(input_dir);
}

void userRequest(char request[],int *totalTravelReqests,int *acceptedRequests,int *rejectedRequests, requestList *listOfRequests,int pidArray[],int fdboard[][2], bloomList (*bloomFiltersOfMonitors)[],stringList countriesOfMonitors[]){
  // implemets the command given by the user
  char function[20];
  sscanf (request,"%s \n",function);  // get the command name
  if(strcmp(function,"/travelRequest")==0){
    char citizenID[20],date[15],countryFrom[50],countryTo[50],virusName[50],wrongInput[50];
    int read=sscanf(request,"%s %s %s %s %s %s %s\n",function,citizenID,date,countryFrom,countryTo,virusName,wrongInput);
    if(read==6){
      if(!validDate(date)){
        printf("Error: Invalid date\n");
        return;
      }
      // call the travelRequest function (travelMonitorFunctions.c) in order to execute the request
      int accepted = travelRequest(citizenID,date,countryFrom,countryTo,virusName,fdboard,(*bloomFiltersOfMonitors),countriesOfMonitors);
      if(accepted == -1){return;} // if the country that the user gave does not exist in the "system"
      (*totalTravelReqests) = (*totalTravelReqests) + 1;  // increase the total request counter
      if(accepted)  // if the request was accepted
        (*acceptedRequests) = (*acceptedRequests) + 1;
      else  // if the request was rejected
        (*rejectedRequests) = (*rejectedRequests) + 1;
      (*listOfRequests) = insertRequestToList((*listOfRequests),virusName,date,countryTo,accepted);
    }else{
      printf("Wrong input.\n");
    }
    return;
  }
  if(strcmp(function,"/addVaccinationRecords")==0){
    char country[50],wrongInput[50];
    int read=sscanf(request,"%s %s %s\n",function,country,wrongInput);
    if(read==2){
      // call the addVaccinationRecords function (travelMonitorFunctions.c) in order to execute the request
      addVaccinationRecords(country,pidArray,fdboard,bloomFiltersOfMonitors,countriesOfMonitors);
    }else{
      printf("Wrong input.\n");
    }
    return;
  }
  if(strcmp(function,"/travelStats")==0){
    char virusName[50],date1[50],date2[50],country[50],wrongInput[50];
    int read=sscanf(request,"%s %s %s %s %s %s\n",function,virusName,date1,date2,country,wrongInput);
    // call the getRequestStats function (travelMonitorFunctions.c) in order to execute the request
    if(read==5){
      if(validDate(date1) && validDate(date2))
        getRequestStats((*listOfRequests),virusName,date1,date2,country);
      else
        printf("Error: Invalid dates\n");
    }else if(read==4){
      if(validDate(date1) && validDate(date2))
        getRequestStats((*listOfRequests),virusName,date1,date2,NULL);
      else
        printf("Error: Invalid dates\n");
    }else{
      printf("Wrong input.\n");
    }
    return;
  }
  if(strcmp(function,"/searchVaccinationStatus")==0){
    char citizenId[50],wrongInput[50];
    int read=sscanf(request,"%s %s %s\n",function,citizenId,wrongInput);
    // call the searchVaccinationStatus function (travelMonitorFunctions.c) in order to execute the request
    if(read==2){
      searchVaccinationStatus(citizenId,fdboard);
    }else{
      printf("Wrong input.\n");
    }
    return;
  }
  printf("Error: Unknown query\n");
}

void killChildren(int pidArray[]){  // send a SIGKILL to every children/monitor
  for(int i=0;i<numMonitors;i++){
    kill(pidArray[i],SIGKILL);
  }
}


void writeLogFile(stringList countries,int totalTravelReqests,int acceptedRequests,int rejectedRequests, stringList *countriesOfMonitors){
  // create the file name: log_file.xxx where xxx is the process id of the travelMonitor
  char fileName[100];
  char tempBuff[]="log_file.";
  char pidBuff[20];
  sprintf(pidBuff,"%d",getpid());
  strcpy(fileName,tempBuff);
  strcat(fileName,pidBuff);
  int fileD;
  if( (fileD = open(fileName, O_WRONLY|O_CREAT|O_TRUNC,0777)) == -1 ){ perror("open: "); exit(-1); }  // open/create the file

  for(int i=0;i<numMonitors;i++){ // write all the country names to the log file
    stringList templist= countriesOfMonitors[i]; // get the countries of every monitor
    while(templist!=NULL){  // write every country to the log file
      if( write(fileD,templist->string,strlen(templist->string)) < strlen(templist->string) ){ perror("write: "); close(fileD); exit(-1); }
      if( write(fileD,"\n",1) < 1 ){ perror("write: "); close(fileD); exit(-1); }
      templist=templist->next;
    }
  }
  char tempBuff2[100];
  // write the requests counters to the log file
  sprintf(tempBuff2,"TOTAL TRAVEL REQUESTS %d\n",totalTravelReqests);
  if( write(fileD,tempBuff2,strlen(tempBuff2)) < strlen(tempBuff2) ){ perror("write: "); close(fileD); exit(-1); }
  sprintf(tempBuff2,"ACCEPTED %d\n",acceptedRequests);
  if( write(fileD,tempBuff2,strlen(tempBuff2)) < strlen(tempBuff2) ){ perror("write: "); close(fileD); exit(-1); }
  sprintf(tempBuff2,"REJECTED %d\n",rejectedRequests);
  if( write(fileD,tempBuff2,strlen(tempBuff2)) < strlen(tempBuff2) ){ perror("write: "); close(fileD); exit(-1); }
  close(fileD); // finally close the file
}


void restoreChildren(int (*pidArray)[],int (*fdboard)[][2], stringList *countriesOfMonitors,bloomList (*bloomFiltersOfMonitors)[], char input_dir[]){
  int pid,status;
  while(( pid = waitpid(-1,&status,WNOHANG))>0){  // for every children that changed state
    if((WIFEXITED(status) || WIFSIGNALED(status)) && !terminated){  // if it was terminated
      int i=-1;
      for(int j=0;j<numMonitors;j++){ // find which monitor has terminated
        if(pid==(*pidArray)[j]){
          i=j;
          break;
        }
      }
      if(i==-1){continue;}

      close((*fdboard)[i][WRITE]);  // close the pipes of this monitor
      close((*fdboard)[i][READ]);
      // create the pipe names in order to pass them as parameter to the new child/monitor
      char fifoNameRead[100];
      sprintf(fifoNameRead,"CHILD%dREAD",i);
      char fifoNameWrite[100];
      sprintf(fifoNameWrite,"CHILD%dWRITE",i);

      int newPid=fork();  //create the new monitor
      if(newPid==-1){ perror("pid: "); exit(-1); }  // if fork failed
      if(newPid==0){  // child process
        char *buffer[]={"./Monitor",NULL};    //convert the name of the executable into a string so as to be given as an argument to execl
        int err=execl(buffer[0],buffer[0],fifoNameRead,fifoNameWrite,(char *)NULL);   // call the monitor executable with the 2 pipes as parameters
        printf("execl error with exit code: %d\n",err); // if execl failed
        perror("execl");
      }else{  // parent process
        (*pidArray)[i]=newPid;  // store the new pid
        // open the reading pipe and store the file descriptor in to the array.
        if( ((*fdboard)[i][READ] = open(fifoNameRead,O_RDONLY)) < 0){ perror("open: "); exit(-1); }
          // open the writing pipe and store the file descriptor in to the array.
        if( ((*fdboard)[i][WRITE] = open(fifoNameWrite,O_WRONLY)) < 0){ perror("open: "); exit(-1); }
        // send the buffSize and the bloom filter size
        if( write((*fdboard)[i][WRITE],&buffSize,4) == -1 ){ perror("write: "); exit(-1); }  // send the buffSize
        sendInt((*fdboard)[i][WRITE],bloomSize); // send the bloomSize
        stringList countries = countriesOfMonitors[i];
        while(countries!=NULL){ // send to the new monitor the countries that the old monitor was responsible for
          char tempBuff[100];
          strcpy(tempBuff,input_dir);
          strcat(tempBuff,"/");
          strcat(tempBuff,countries->string); // create the directory path that corresponds to the country
          sendMessage((*fdboard)[i][WRITE],tempBuff); // send the directory path
          countries=countries->next;
        }
        char endbuff[]="ENDOFCOUNTRIES";
          // after assinging all the counties to the monitors, send a message to the monitor in order to show that the country assignment has finished
        sendMessage((*fdboard)[i][WRITE],endbuff);
        // delete the bloomFilter list of the old monitor
        deleteBloomList((*bloomFiltersOfMonitors)[i]);
        (*bloomFiltersOfMonitors)[i]=initializeBloomList(); // initialize a new list
        while(1){ // start receiving the bloomFilters from the new monitor
          char *virusName=readMessage((*fdboard)[i][READ]);   //read the message
          if(strcmp(virusName,"ENDOFBLOOMFILTERS")==0){ // that means that the monitor has sent all the bloomFilters
            free(virusName);
            break;
          }else{
            bloomFilter bf = readBloom((*fdboard)[i][READ]);  // read the bloom filter that corresponds to the virusName
            // add the bloom filter to the bloom filter list of this monitor
            (*bloomFiltersOfMonitors)[i] = insertBloomToList((*bloomFiltersOfMonitors)[i],virusName,bf);
          }
        }
        // after reading all the bloom filters, the new monitor is "ready" so return from the signal handler and continue the travelMonitor opperations
      }

    }
  }
}

void IntQuitHandler(int signum){  // signal handler for SIGINT and SIGQUIT signals
    terminated=1;
}

void SigchldHandler(int signum){  // signal handler for SIGCHLD signals
  childTerminated=1;
}
