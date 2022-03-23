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
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <arpa/inet.h>
#include "bloomFilter/bloomFilter.h"
#include "readFiles.h"
#include "database/database.h"
#include "socketReadWrite.h"
#include "threadFunctions.h"

# define READ 0
# define WRITE 1
#define threadError(s, e) fprintf(stderr, "%s: %s\n", s, strerror(e))

int buffSize; // the size of the buffer (given by the user and sent from father through the socket)
// every time we send/receive a message through a socket we write/read at most buffSize bytes (see socketReadWrite.c file)
int bloomSize;  // the size of every bloom filter (given by the user and sent from father through the socket)


pthread_mutex_t mtx;  // mutex for cond_nonempty and cond_nonfull condition variables
pthread_mutex_t updateData; // mutex for synchronizing threads when updating the database
pthread_mutex_t endflag;  // mutex for changing the endThreads global variable
pthread_mutex_t totalFilesMutex;  // mutex for changing the totalFiles global variable
pthread_cond_t cond_nonempty; // condition variable used in the cyclic buffer. Designates that there is at least one fileName at the buffer
pthread_cond_t cond_nonfull;  // condition variable used in the cyclic buffer. Designates that the buffer is full

pthread_mutex_t parsingMtx; // mutex for parsingEnded condition variable
pthread_cond_t parsingEnded;  // condition variable used so as the main thread waits until the threads finish reading all the files and updating the database

cyclicBuffer cycBuff;  //the buffer used in order to write the file names that the threads will read (defined in threadFunctions.c)
int cyclisticBufferSize;  // the size of the cyclic buffer (given by the user)
int endThreads; // flag that designated that the threads should terminate (when /exit is given by the user)
Data globalDatabase;  // the database (same as project2)
int totalFiles; // the number of files that should be read by the threads

void writeLogFile(stringList ,int ,int ,int );
void readNewFiles(stringList ,char *,Data ,stringList *,int );


int main(int argc, char* argv[]){

  int totalTravelReqests=0; // stores the total number of request that the monitor received by the travelMonitor (father)
  int acceptedRequests=0; // stores how many of the requests were accepted
  int rejectedRequests=0;   // stores how many of the requests were rejected

  int port,numThreads;
  // get the command line arguments
  port = atoi(argv[2]);
  numThreads = atoi(argv[4]);
  buffSize = atoi(argv[6]);
  cyclisticBufferSize = atoi(argv[8]);
  bloomSize = atoi(argv[10]);

  stringList countries=initializeStringList();  // list that stores all the countries that this monitor is responsible for
  for(int i=11;i<argc;i++){ // get the countries directories from the command line
    countries = insertString(countries,argv[i]);
  }

  int sockMain,sock;
  struct sockaddr_in server,client;
  socklen_t clientlen= (socklen_t)sizeof(client);

  struct sockaddr *serverptr=(struct sockaddr *)&server;
  struct sockaddr *clientptr=(struct sockaddr *)&client;

  if ((sockMain = socket(AF_INET, SOCK_STREAM, 0)) < 0) {perror("socket"); exit(-1);} // create a socket
  int flagEnable=1;
  if (setsockopt(sockMain, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &flagEnable, sizeof(flagEnable))){  // set options in order to reuse the port if it is already used
        perror("setsockopt");
        exit(-1);
  }

  struct hostent *rem;
  char hostbuffer[256];
  if((gethostname(hostbuffer, sizeof(hostbuffer)))<0){perror("gethostname"); exit(-1);} // find the host name
  if ((rem = gethostbyname(hostbuffer)) == NULL) {  // find the info of the host
   herror("gethostbyname"); exit(1);
  }

  server.sin_family = AF_INET;       // Internet domain
  memcpy(&server.sin_addr, rem->h_addr, rem->h_length);
  server.sin_port = htons(port);      // the given port

  if (bind(sockMain, serverptr, sizeof(server)) < 0) {perror("bind"); exit(-1);}

  if (listen(sockMain, 5) < 0){perror("listen"); exit(-1);}

  if ((sock = accept(sockMain, clientptr, &clientlen)) < 0) {perror("accept"); exit(-1);} // accept the connection by the travelMonitorClient
  // sock now contains the descriptor for "speaking" with the travelMonitorClient

  cycBuff = initializeCycBuf(); // initialize the cyclic buffer
  // initialize all the mutexes and the condition variables
  pthread_mutex_init(&mtx, 0);
  pthread_mutex_init(&updateData, 0);
  pthread_mutex_init(&endflag, 0);
  pthread_mutex_init(&totalFilesMutex, 0);
	pthread_cond_init(&cond_nonempty, 0);
	pthread_cond_init(&cond_nonfull, 0);
  pthread_mutex_init(&parsingMtx, 0);
	pthread_cond_init(&parsingEnded, 0);
  endThreads = 0;

  pthread_t *threadIds;
  int e;
  // create an array that will store all the thread id's
  if ((threadIds = malloc(numThreads * sizeof(pthread_t))) == NULL) {perror("malloc"); exit(-1);}

  for(int i=0;i<numThreads;i++){  // create all the threads (no arguments are given to them)
    // threads execute obtainFiles function defined in threadFunctions.c
    if((e = pthread_create(threadIds+i, NULL, obtainFiles,NULL)) != 0) {threadError("pthread_create",e); exit(-1);}
  }

  stringList filesSeen=initializeStringList();  // list that stores all the files that the monitor have read
  Data database=initializeData(); // all the data will be stored here. bloomFilters, skipLists, hashtable etc. Same as project1
  globalDatabase = database;
  totalFiles=0;
  stringList templist=countries;
  while(templist!=NULL){  // for every country that was assigned to this monitor
     // find the files of every country directory and write them to the cyclic buffer so as the threads read them
    readFilesOfDir(templist->string,database,&filesSeen);
    templist=templist->next;
  }

  pthread_mutex_lock(&parsingMtx);
  while(totalFiles!=0){ // wait until all the files are read and the database is "ready"
    pthread_cond_wait(&parsingEnded, &parsingMtx);
  }
  pthread_mutex_unlock(&parsingMtx);


  // after threads read all the files and insert all the records to the database, send all the bloom filters to the father (travelMonitorClient)
  sendBloomFilters(sock,database); // sendBloomFilters is implemented in database.c file

  struct pollfd pfds[1];  // use poll to monitor the socket
  pfds[0].fd=sock;  // add the socket descriptor of the equivelent socket to the poll array
  pfds[0].events=POLLIN;      // we are only intersted in reading so we add only POLLIN in the inrested events of poll

  while(1){
    int retval = poll(pfds,1,10);
    if(retval<0){ //if poll is interrupted by a signal, continue and redo poll
        continue;
    }
    if((pfds[0].revents&POLLIN)){ // if something was written to the socket from the father
        char *request=readMessage(sock);  // read the message
        if(strcmp(request,"/exit")==0){ // if /exit was given by the user
          free(request);
          break;  // stop looping
        }
        if(strcmp(request,"/travelRequest")==0){  // if /travelRequest was read
          char *id=readMessage(sock); // read the id of the citizen
          char *virus=readMessage(sock);  // read the virusName
          // search in the database to see if the citizen is vaccinated
          char *vaccinationDate=vaccineStatus(database,id,virus);
          totalTravelReqests++; // increase the total request counter
          if(vaccinationDate==NULL){  // if NULL was returned from vaccineStatus, the citizen is not vaccinated
            sendMessage(sock,"NO");  // send NO to the travelMonitor
            rejectedRequests++; // increase the rejected counter
          }else{  // if the citizen is vaccinated
            sendMessage(sock,"YES"); // send YES to the travelMonitor
            sendMessage(sock,vaccinationDate); //also send the vaccination date
            char *accepted=readMessage(sock); // wait a message from travelMonitor
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
          char *id=readMessage(sock); // read the citizen id
          // send all the info that we have for this citizen in the database (if it exists)
          vaccineStatusAll(database,id,sock);  // vaccineStatusAll is implemented in the database.c file
          free(id);
        }
        if(strcmp(request,"/addVaccinationRecords")==0){  // if /addVaccinationRecords was read
          char *country=readMessage(sock); // read the country that the new files were added
          // find the new files and write them to the cyclic buffer so as the threads read them and update the database
          readNewFiles(countries,country,database,&filesSeen,sock);
          free(country);
        }
        free(request);
    }

  }
  // when /exit was given by the user
  writeLogFile(countries,totalTravelReqests,acceptedRequests,rejectedRequests); // write to logFile

  pthread_mutex_lock(&endflag);
  endThreads=1; // designate that the threads should terminate
  pthread_mutex_unlock(&endflag);

  pthread_cond_broadcast(&cond_nonempty); // "wake" all the threads

  void *ignoreReturn;
  for(int i=0;i<numThreads;i++){  // wait for the threads to terminate
    if((e = pthread_join(*(threadIds+i), &ignoreReturn)) != 0) {threadError("pthread_join",e); exit(-1);}
  }
  // free up all the dynamically allocated memmory
  free(threadIds);
  deleteStringList(filesSeen);
  deleteStringList(countries);
  deleteData(database);
  destroyCycBuf();
  // close the socket
  close(sock);
  close(sockMain);
  // destroy all the mutexes and the condition variables
  pthread_mutex_destroy(&mtx);
  pthread_mutex_destroy(&updateData);
  pthread_mutex_destroy(&endflag);
  pthread_mutex_destroy(&totalFilesMutex);
  pthread_mutex_destroy(&parsingMtx);
  pthread_cond_destroy(&cond_nonempty);
	pthread_cond_destroy(&cond_nonfull);
  pthread_cond_destroy(&parsingEnded);
  exit(0);
}

void readNewFiles(stringList countries,char *givenCountry,Data database,stringList *filesSeen,int fdWrite){
  int foundNewFile=0;
  stringList templist=countries;
  while(templist!=NULL){  // for every country/directory that this monitor is responsible for
    char *tempBuff=strdup(templist->string);
    char *directoryName=basename(tempBuff); // get the name of the country (= the name of the directory)
    if(strcmp(directoryName,givenCountry)==0){  // if we found the given country
      // check if there is a new file, and if it exists write it to the cyclic buffer so as the threads read id
      readNewFilesOfDir(templist->string,database,filesSeen,&foundNewFile); // readNewFilesOfDir is implemented in readDirs.c
    }
    free(tempBuff);
    templist=templist->next;
  }
  // wait in for the threads to finish reading the files and updating the database
  pthread_mutex_lock(&parsingMtx);
  while(totalFiles!=0){
    pthread_cond_wait(&parsingEnded, &parsingMtx);
  }
  pthread_mutex_unlock(&parsingMtx);


  if(foundNewFile){ // if at least one new file was read
    sendBloomFilters(fdWrite,database); // send the updated bloomFilters to the travelMonitor
  }else{  // if no new files were found, dont send the bloomFilters as they dont have changed
    char tempBuff[]="ENDOFBLOOMFILTERS";
    sendMessage(fdWrite,tempBuff);
  }
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
