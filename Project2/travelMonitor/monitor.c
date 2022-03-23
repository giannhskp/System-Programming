#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <poll.h>
#include "bloomFilter/bloomFilter.h"
#include "readDirs.h"
#include "database/database.h"
#include "pipeReadWrite.h"

# define READ 0
# define WRITE 1
int buffSize; // the size of the buffer (given by the user and sent from father through the pipe)
// every time we send/receive a message through a pipe we write/read at most buffSize bytes (see pipeReadWrite.c file)
int bloomSize;  // the size of every bloom filter (given by the user and sent from father through the pipe)
int writeLog=0; // flag that shows if a SIGINT or SIGQUIT has been received
int readNewFile=0;  // flag that shows if a USR1 signal has been received

void Usr1Handler();
void IntQuitHandler();
void writeLogFile(stringList ,int ,int ,int );
void readNewFiles(stringList ,Data ,stringList *,int );


int main(int argc, char* argv[]){
  struct sigaction signalAction;  // handling the signals that we receive using sigaction
  signalAction.sa_handler = Usr1Handler;  // set our function as the handler (implemented at the bottom of the file)
  sigemptyset(&signalAction.sa_mask);   //initialize
  signalAction.sa_flags = SA_RESTART;
  sigaction(SIGUSR1, &signalAction, NULL);  // handle the USR1 signals
  signalAction.sa_handler = IntQuitHandler;  // set our function as the handler (implemented at the bottom of the file)
  sigemptyset(&signalAction.sa_mask);   //initialize
  signalAction.sa_flags = SA_RESTART;
  sigaction(SIGINT, &signalAction, NULL); // handle the SIGINT signals
  sigaction(SIGQUIT, &signalAction, NULL);  // handle the SIGQUIT signals

  int totalTravelReqests=0; // stores the total number of request that the monitor received by the travelMonitor (father)
  int acceptedRequests=0; // stores how many of the requests were accepted
  int rejectedRequests=0;   // stores how many of the requests were rejected


  int fd[2];
  // open the writing pipe and store the file descriptor in to the array.
  if( (fd[WRITE] = open(argv[1],O_WRONLY)) < 0){ perror("open: "); return -1; }
  // open the reading pipe and store the file descriptor in to the array.
  if( (fd[READ] = open(argv[2],O_RDONLY)) < 0){ perror("open: "); return -1; }

  if(read(fd[READ],&buffSize,4) < 0 ){ perror("read: "); return -1; } //read the buffSize
  bloomSize = readInt(fd[READ]);   //read the bloomSize

  stringList countries=initializeStringList();  // list that stores all the countries that this monitor is responsible for
  stringList filesSeen=initializeStringList();  // list that stores all the files that the monitor have read
  int flag=1;
  while(flag){  // read all the countries that have been assigned to this monitor from the travelMonitor
    char *received=readMessage(fd[READ]); // read the country / directory path
    if(strcmp(received,"ENDOFCOUNTRIES")==0){ // if ENDOFCOUNTRIES was read, that means that all countries were read
      flag=0;
    }else{  // if a country was read, add it to the country list
      countries = insertString(countries,received);
    }
    free(received);
  }
  if(countries==NULL){  // if no countries were assigned to this monitor, then it terminates
    close(fd[READ]);
    close(fd[WRITE]);
    exit(0);
  }
  Data database=initializeData(); // all the data will ber stored here. bloomFilters, skipLists, hashtable etc. Same as project1
  stringList templist=countries;
  while(templist!=NULL){  // for every country that was assigned to this monitor
    readFilesOfDir(templist->string,database,&filesSeen); // read the files of this country, and add the records of each file to the database
    templist=templist->next;
  }
  // after reading all the files and inserting all the records to the database, send all the bloom filters to the father (travelMonitor)
  sendBloomFilters(fd[WRITE],database); // sendBloomFilters is implemented in database.c file

  struct pollfd pfds[1];  // use poll to monitor the pipe
  pfds[0].fd=fd[READ];  // add the pipe descriptor of the equivelent pipe to the poll array
  pfds[0].events=POLLIN;      // we are only intersted in reading so we add only POLLIN in the inrested events of poll

  while(1){
    int retval = poll(pfds,1,10);
    if(retval<0){ //if poll is interrupted by a signal, continue and redo poll
        continue;
    }
    if((pfds[0].revents&POLLIN)){ // if something was written to the pipe from the father
        char *request=readMessage(fd[READ]);  // read the message
        if(strcmp(request,"/travelRequest")==0){  // if /travelRequest was read
          char *id=readMessage(fd[READ]); // read the id of the citizen
          char *virus=readMessage(fd[READ]);  // read the virusName
          // search in the database to see if the citizen is vaccinated
          char *vaccinationDate=vaccineStatus(database,id,virus);
          totalTravelReqests++; // increase the total request counter
          if(vaccinationDate==NULL){  // if NULL was returned from vaccineStatus, the citizen is not vaccinated
            sendMessage(fd[WRITE],"NO");  // send NO to the travelMonitor
            rejectedRequests++; // increase the rejected counter
          }else{  // if the citizen is vaccinated
            sendMessage(fd[WRITE],"YES"); // send YES to the travelMonitor
            sendMessage(fd[WRITE],vaccinationDate); //also send the vaccination date
            char *accepted=readMessage(fd[READ]); // wait a message from travelMonitor
            // travel monitor informs the monitor in order to know if the request was finally accepted or not
            if(strcmp(accepted,"ACCEPTED")==0)  // if the request was accepted
              acceptedRequests++; // increase the accepted counter
            else  // if the request was rejected
              rejectedRequests++; // increase the rejected counter
            free(accepted);
          }
          free(id);
          free(virus);
        }
        if(strcmp(request,"/searchVaccinationStatus")==0){  // if /searchVaccinationStatus was read
          char *id=readMessage(fd[READ]); // read the citizen id
          // send all the info that we have for this citizen in the database (if it exists)
          vaccineStatusAll(database,id,fd[WRITE]);  // vaccineStatusAll is implemented in the database.c file
          free(id);
        }
        free(request);
    }

    if(readNewFile){  // if USR1 signal was received
      // the a new file was placed inside a directory
      readNewFiles(countries,database,&filesSeen,fd[WRITE]);  // find the file and read it
      readNewFile=0;
    }
    if(writeLog){ // if SIGINT or SIGQUIT was received
      writeLogFile(countries,totalTravelReqests,acceptedRequests,rejectedRequests); // write to logFile
      writeLog=0;
    }
  }

  deleteStringList(filesSeen);
  deleteStringList(countries);
  deleteData(database);
  close(fd[READ]);
  close(fd[WRITE]);
  exit(0);
}

void readNewFiles(stringList countries,Data database,stringList *filesSeen,int fdWrite){
  int foundNewFile=0;
  stringList templist=countries;
  while(templist!=NULL){  // for every country/directory that this monitor is responsible for
    // check if there is a new file, and if it exists read it
    readNewFilesOfDir(templist->string,database,filesSeen,&foundNewFile); // readNewFilesOfDir is implemented in readDirs.c
    templist=templist->next;
  }
  if(foundNewFile){
    sendBloomFilters(fdWrite,database); // send the updated bloomFilters to the travelMonitor
  }else{  // if no new files were found, dont send the bloomFilters as they dont have changed
    char tempBuff[]="ENDOFBLOOMFILTERS";
    sendMessage(fdWrite,tempBuff);
  }
}

void Usr1Handler(int signum){ // signal handler for USR1 signals
    readNewFile=1;
}

void IntQuitHandler(int signum){  // signal handler for SIGINT and SIGQUIT signals
  writeLog=1;
}

void writeLogFile(stringList countries,int totalTravelReqests,int acceptedRequests,int rejectedRequests){
    // create the file name: log_file.xxx where xxx is the process id of the monitor
  char fileName[100];
  char tempBuff[]="log_file.";
  char pidBuff[20];
  sprintf(pidBuff,"%d",getpid());
  strcpy(fileName,tempBuff);
  strcat(fileName,pidBuff);
  int fileD;
  if( (fileD = open(fileName, O_WRONLY|O_CREAT|O_TRUNC,0777)) == -1 ){ perror("open: "); exit(-1); }   // open/create the file
  stringList templist=countries;
  while(templist!=NULL){  // for every country/directory that this monitor is responsible for
    char *tempBuff=strdup(templist->string);
    char *countryName=basename(tempBuff); // get the countryName - name of the directory
    // and write it to the logFile
    if( write(fileD,countryName,strlen(countryName)) < strlen(countryName) ){ perror("write: "); close(fileD); exit(-1); }
    if( write(fileD,"\n",1) < 1 ){ perror("write: "); close(fileD); exit(-1); }
    free(tempBuff);
    templist=templist->next;
  }
  // write the requests counters to the log file
  char tempBuff2[100];
  sprintf(tempBuff2,"TOTAL TRAVEL REQUESTS %d\n",totalTravelReqests);
  if( write(fileD,tempBuff2,strlen(tempBuff2)) < strlen(tempBuff2) ){ perror("write: "); close(fileD); exit(-1); }
  sprintf(tempBuff2,"ACCEPTED %d\n",acceptedRequests);
  if( write(fileD,tempBuff2,strlen(tempBuff2)) < strlen(tempBuff2) ){ perror("write: "); close(fileD); exit(-1); }
  sprintf(tempBuff2,"REJECTED %d\n",rejectedRequests);
  if( write(fileD,tempBuff2,strlen(tempBuff2)) < strlen(tempBuff2) ){ perror("write: "); close(fileD); exit(-1); }
  close(fileD);
}
