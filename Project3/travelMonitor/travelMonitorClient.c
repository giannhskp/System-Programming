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
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <arpa/inet.h>
#include "bloomFilter/bloomFilter.h"
#include "readDirs.h"
#include "database/database.h"
#include "linkedLists/bloomList.h"
#include "socketReadWrite.h"
#include "travelMonitorFunctions.h"
#include "linkedLists/requestList.h"
#include "errorChecking/errorChecking.h"

# define INITIAL_PORT 9003

# define READ 0
# define WRITE 1
int buffSize;   // the size of the buffer (given by the user)
// every time we send/receive a message through a socket we write/read at most buffSize bytes (see socketReadWrite.c file)
int bloomSize;   // the size of every bloom filter (given by the user)
int numMonitors;  // the number of monitors that will be created (given by the users)

int terminated=0; // flag that shows if a SIGINT or SIGQUIT has been received

void writeLogFile(stringList ,int ,int ,int ,stringList *);
void userRequest(char [], int *,int *,int *,requestList *,int [], int [], bloomList (*)[],stringList []);


int main(int argc, char* argv[]){
  if(argc<12){
    printf("Wrong input\nUsage: ./travelMonitorClient –m numMonitors -b socketBufferSize -c cyclicBufferSize -s sizeOfBloom -i input_dir -t numThreads\n");
    return 0;
  }
  char *input_dir; // the path of the input directory given by the user
  int numThreads=5;
  int cyclisticBufferSize=10;

  int count=1;
  int read1=0,read2=0,read3=0,read4=0,read5=0,read6=0;  // for wrong input check
  while(count<=11){  //read  the arguments of the command line
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
    if(strcmp("-c",argv[count])==0){  // -s for sizeOfBloom (bloomSize)
      cyclisticBufferSize=atoi(argv[count+1]);
      read5=1;
      count+=2;
      continue;
    }
    if(strcmp("-t",argv[count])==0){  // -s for sizeOfBloom (bloomSize)
      numThreads=atoi(argv[count+1]);
      read6=1;
      count+=2;
      continue;
    }
    count+=2;
  }
  if( !read1 || !read2 || !read3 || !read4 || !read5 || !read6){    // check if all arguments were read
    printf("Wrong input\n");
    return 0;
  }


  int totalTravelReqests=0; // stores the total number of request that the travelMonitor received by the user
  int acceptedRequests=0; // stores how many of the requests were accepted
  int rejectedRequests=0; // stores how many of the requests were rejected

  bloomList bloomFiltersOfMonitors[numMonitors];  // array of lists that contains all the bloomFilters of a monitor
  stringList countriesOfMonitors[numMonitors]; // array of lists that contains all the countries that a monitor is "responsible" for
  stringList countryPathsOfMonitors[numMonitors]; // array of lists that contains all the directory paths that a monitor is "responsible" for
  int countryCounterOfMonitors[numMonitors];  // array that counts how many countries a monitor is responsible for
  int pidArray[numMonitors];  // an array that stores the process id of every monitor
  requestList listOfRequests=initializeRequestList(); // a list that stores info for every request received by the user

  int childSockets[numMonitors];  // array that stores the socet descriptor of every child process
  int port;
  struct sockaddr_in server;
  struct sockaddr *serverptr=(struct sockaddr *)&server;
  port = INITIAL_PORT;  // the port of the socket connection with the first child
  // for every child the port is increase by one, because every child should have a different port as it creates it's own server

  for(int i=0;i<numMonitors;i++){ // initialize the lists
    countriesOfMonitors[i]=initializeStringList();
    countryPathsOfMonitors[i]=initializeStringList();
    countryCounterOfMonitors[i]=0;
  }


  stringList countries=readDirs(input_dir); // read the input directory and get the path of every directory
  // country list is sorted by country name
  stringList templist=countries;
  int roundRobinCounter=0;
  int countryCounter=0; // counts the number of countries/directories
  // ROUND ROBIN distribution of the countries to the monitors
  while(templist!=NULL){  // for every country/directory
    countryCounterOfMonitors[roundRobinCounter]=countryCounterOfMonitors[roundRobinCounter]+1;  // increase the country counter
    countryPathsOfMonitors[roundRobinCounter] = insertString(countryPathsOfMonitors[roundRobinCounter],templist->string); // add the path to the list
    char *tempBuff=strdup(templist->string);
    char *directoryName=basename(tempBuff); // get the name of the country (= the name of the directory)
    // and insert the country to the country list that corresponds to this monitor
    countriesOfMonitors[roundRobinCounter] = insertString(countriesOfMonitors[roundRobinCounter],directoryName);
    free(tempBuff);
    roundRobinCounter = (roundRobinCounter+1)%numMonitors;
    countryCounter++;
    templist=templist->next;
  }

  if(countryCounter<numMonitors)  // if the number of countries is less than the number of monitors
    numMonitors=countryCounter; // some of the monitors are not used, so only keep as many monitors as the number of countries
  deleteStringList(countries);



  for(int i=0;i<numMonitors;i++){  // loop that creates the childs (monitors), in every loop a child is created

    int pid=fork();
    if(pid==-1){ perror("fork: "); exit(-1); }

  	if (pid==0){ //child process
      // define the arguments
      int numArgs = 10 + countryCounterOfMonitors[i];
      char *args[numArgs+2];
      char tempbuff[50]="./monitorServer";
      args[0]=strdup(tempbuff);
      args[1]="-p";
      sprintf(tempbuff,"%d",port+i);
      args[2]=strdup(tempbuff);
      args[3]="-t";
      sprintf(tempbuff,"%d",numThreads);
      args[4]=strdup(tempbuff);
      args[5]="-b";
      sprintf(tempbuff,"%d",buffSize);
      args[6]=strdup(tempbuff);
      args[7]="-c";
      sprintf(tempbuff,"%d",cyclisticBufferSize);
      args[8]=strdup(tempbuff);
      args[9]="-s";
      sprintf(tempbuff,"%d",bloomSize);
      args[10]=strdup(tempbuff);
      stringList pathList =countryPathsOfMonitors[i];
      for(int j=11;j<=numArgs;j++){ // add as arguments the countries that the monitor is responsible for
        args[j]= strdup(pathList->string);
        pathList = pathList->next;
      }
      args[numArgs+1]=NULL;
      int err=execvp(args[0],args);
      printf("execl error with exit code: %d\n",err); // if execl failed
      perror("execl");
  	}else{ // father process
      // initialize the data stored for this monitor
      bloomFiltersOfMonitors[i]=initializeBloomList();  // initialize the list of bloomFilters of this child
      pidArray[i]=pid;  // store the process id of this children
    }
  }
  struct hostent *rem;
  char hostbuffer[256];
  if((gethostname(hostbuffer, sizeof(hostbuffer)))<0){perror("gethostname"); exit(-1);} // find the host name
  if ((rem = gethostbyname(hostbuffer)) == NULL) {  // find the info of the host
   herror("gethostbyname"); exit(1);
  }

  for(int i=0;i<numMonitors;i++){ // for every child/monitor
    int tempsock;
    if ((tempsock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {perror("bind"); exit(-1);} // create a socket
    server.sin_family = AF_INET;       /* Internet domain */
    memcpy(&server.sin_addr, rem->h_addr, rem->h_length);
    server.sin_port = htons(port+i);  // every monitor has a different port in increasing order (starting from INITIAL_PORT)
    int returnCon;
    do{
      returnCon = connect(tempsock, serverptr, sizeof(server)); // and connect to the socket/server of the monitor
    }while(returnCon<0);  // while loop because the monitos may delay creating the server
    childSockets[i]=tempsock; // store the descriptor of the socket for this child/monitor
  }

  // at this point we have connected to all the sockets of the monitorServers

  int ended[numMonitors]; // flag array that contains if the reading from every socket if finished
  struct pollfd pfds[numMonitors];  // use poll to monitor all the sockets
  // each place of the poll board corresponds to the socket of the equivelent children
  for(int i=0;i<numMonitors;i++){
    ended[i]=0;   // initialize the end board with zero as we have not started reading yet
    pfds[i].fd=childSockets[i];  // add the socket descriptor of the equivelent socket to the poll array
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
    for(int i=0;i<numMonitors;i++){ // for every monitor, check if it has written to the socket
      if((pfds[i].revents&POLLIN)){ //if something is written in this socket
          char *virusName=readMessage(childSockets[i]);  //read the message
          if(strcmp(virusName,"ENDOFBLOOMFILTERS")==0){ // that means that the monitor has sent all the bloomFilters
            free(virusName);
            ended[i]=1; // the monitor has ended sending the bloomFilters
          }else{
            bloomFilter bf = readBloom(childSockets[i]); // read the bloom filter that corresponds to the virusName
            // add the bloom filter to the bloom filter list of this monitor
            bloomFiltersOfMonitors[i] = insertBloomToList(bloomFiltersOfMonitors[i],virusName,bf);

          }
      }
    }
    flag=0;
   //check if we have finished reading from all the sockets
    for(int i=0;i<numMonitors;i++){   //check if all monitors have ended sending the bloom filters
      if(ended[i]==0)
        flag=1;   // if we have not finished reading from at least one socket, loop until one socket is ready for reading
      // if we have finished reading from all the sockets the flag is 0 so the loop (and the reading) is stopped
    }
  }

  printf("All the initializations and the file parsing ended succesfully.\n Give your commands:\n");
  while(1){ // start receiving requests from the user
    char readBuff[500];
    printf(">: ");
    if(fgets(readBuff,500,stdin)==NULL){   //get command that the user gave
      perror("fgets: ");
      return -1;
    }
    // execute the request/command of the user
    userRequest(readBuff,&totalTravelReqests,&acceptedRequests,&rejectedRequests,&listOfRequests,pidArray,childSockets,&bloomFiltersOfMonitors,countriesOfMonitors);
    if(terminated==1)
      break;
  }
  // if /exit command was received from user

  writeLogFile(countries,totalTravelReqests,acceptedRequests,rejectedRequests,countriesOfMonitors);  // write to the logFile
  int wpid,status=0;
  while (( wpid = wait(&status)) > 0);  // wait for all the childs to finish executing

  // free up all the memory that was allocated and close the open socket connections
  deleteRequestList(listOfRequests);
  for(int i=0;i<numMonitors;i++){
    deleteBloomList(bloomFiltersOfMonitors[i]);
    deleteStringList(countriesOfMonitors[i]);
    deleteStringList(countryPathsOfMonitors[i]);
    close(childSockets[i]);
  }
  free(input_dir);
}

void userRequest(char request[],int *totalTravelReqests,int *acceptedRequests,int *rejectedRequests, requestList *listOfRequests,int pidArray[],int fdboard[], bloomList (*bloomFiltersOfMonitors)[],stringList countriesOfMonitors[]){
  // implemets the command given by the user
  char function[20];
  sscanf (request,"%s \n",function);  // get the command name
  if(strcmp(function,"/exit")==0){  // if /exit was given
    terminated=1; // flag in order to stop looping
    terminate(fdboard); // send the /exit command to all the children in order to terminate
    return;
  }
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
