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
#include "database/database.h"
#include "linkedLists/stringList.h"

typedef struct cb{
	char **data;	// string (char *) array that the file names are stored
	int start;	// designates the position that the first file name is written at each time
	int end;	// designates the position that the last file name is written at each time
	int count;	// designated how many file names are written in the buffer at each time
} cyclicBufferStruct;	// the cyclic buffer
typedef cyclicBufferStruct *cyclicBuffer;

extern pthread_mutex_t mtx;	// mutex for cond_nonempty and cond_nonfull condition variables
extern pthread_mutex_t updateData; // mutex for synchronizing threads when updating the database
extern pthread_mutex_t endflag;  // mutex for changing the endThreads global variable
extern pthread_mutex_t totalFilesMutex;  // mutex for changing the totalFiles global variable
extern pthread_cond_t cond_nonempty; // condition variable used in the cyclic buffer. Designates that there is at least one fileName at the buffer
extern pthread_cond_t cond_nonfull;  // condition variable used in the cyclic buffer. Designates that the buffer is full
extern pthread_mutex_t parsingMtx; // mutex for parsingEnded condition variable
extern pthread_cond_t parsingEnded;  // condition variable used so as the main thread waits until the threads finish reading all the files and updating the database
extern cyclicBuffer cycBuff;  //the buffer used in order to write the file names that the threads will read (defined in threadFunctions.c)
extern int endThreads; // flag that designated that the threads should terminate (when /exit is given by the user)
extern int totalFiles; // the number of files that should be read by the threads
extern Data globalDatabase;  // the database (same as project2)
extern int cyclisticBufferSize;  // the size of the cyclic buffer (given by the user)


cyclicBuffer initializeCycBuf() {	// initialize the cyclic buffer
  cyclicBuffer pool = malloc(sizeof(cyclicBufferStruct));
  pool->data = malloc(cyclisticBufferSize*(sizeof(char *)));	// create a string (char *) array of size: cyclisticBufferSize
	pool->start = 0;
	pool->end = -1;
	pool->count = 0;
  return pool;
}

void destroyCycBuf(){
  free(cycBuff->data);
  free(cycBuff);
}

void place(cyclicBuffer pool, char *data) {	// store a file name to the cyclic buffer
	pthread_mutex_lock(&mtx);
	while (pool->count >= cyclisticBufferSize) {	// check if the buffer is full
		// if the buffer is full, wait until it has at least one empty space
		pthread_cond_wait(&cond_nonfull, &mtx);
	}
	pool->end = (pool->end + 1) % cyclisticBufferSize;	// the position that the new file name will be placed
	pool->data[pool->end] = strdup(data);	// store the file name
	pool->count++;	// increase the current files in the cyclic buffer
	pthread_mutex_unlock(&mtx);	// release the mutex
}

void obtain(cyclicBuffer pool) {	// read a file name from the cyclic buffer and insert the records of the file to the database
	char *data = NULL;
	pthread_mutex_lock(&mtx);
	while (pool->count <= 0) {	// check if the buffer is empty
		// if the buffer is emtpy, wait until at least one file name is written to it
		pthread_cond_wait(&cond_nonempty, &mtx);
    pthread_mutex_lock(&endflag);
    if(endThreads==1){	// after "waking up" check if the main thread broadcasted in order to terminate
      pthread_mutex_unlock(&mtx);
      pthread_mutex_unlock(&endflag);
      return;
    }
    pthread_mutex_unlock(&endflag);
	}
	data = pool->data[pool->start];	// read the file name
	pool->start = (pool->start + 1) % cyclisticBufferSize;	// "point" to the next position of the buffer
	pool->count--;	// decrease the current files in the cyclic buffer
	pthread_mutex_unlock(&mtx);	//release the mutex
		// read the records of the file and insert them to the database
		// when inserting a record to the database a mutex is used so as not 2 or more threads access the database at the same time
		// the lock/unlock of the mutex is done inside readCitizenRecordsFile function in the file database.c
		// the critical section is considered the insertion of a signle record to the databse
	 readCitizenRecordsFile(globalDatabase,data,&updateData);	// readCitizenRecordsFile is the same as project2 (defined in database.c)



  pthread_mutex_lock(&totalFilesMutex);
	  totalFiles--;	// decrease the totalFiles counter
	  if(totalFiles==0){	// check if it was the last file to be read
			// if it was, "wake up" the main thread in order to send the bloom filters to the travelMonitorClient (as the reading of the files is done)
	    pthread_cond_signal(&parsingEnded);
	  }
  pthread_mutex_unlock(&totalFilesMutex);
  free(data);
}

void producer(stringList files){	// called for every directory by the main thread. Called by readFilesOfDir function defined in readFiles.c
  stringList templist = files;
  while(templist!=NULL){	// for every file
    place(cycBuff,templist->string);	// place it to the buffer
		pthread_cond_signal(&cond_nonempty);	// signal that a new file name was placed in to the buffer (so it is not empty)
    templist=templist->next;
  }

}

void * obtainFiles(void * ptr){	// the starting point of every thread
  while ( cycBuff->count > 0 || endThreads==0) {	// while the main thread did not change the endThreads flag in order to terminate
    obtain(cycBuff);	// read a file from the buffer an insert it's records to the database
		pthread_cond_signal(&cond_nonfull);	// signal that a file name was read from the buffer (so it is not full)
	}
	pthread_exit(NULL);
}
