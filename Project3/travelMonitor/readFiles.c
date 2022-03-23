#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include "linkedLists/stringList.h"
#include "database/database.h"
#include "threadFunctions.h"

extern int totalFiles;
extern pthread_mutex_t totalFilesMutex;

void readFilesOfDir(char *countryDir,Data database,stringList *filesSeen){
  // read the files inside of a country directory, and write them to the cyclic buffer
  int fileCount=0;  // counts how many files the directory has
  DIR 	*dp;
	struct 	dirent *direntp;
  stringList filesOfDir = initializeStringList(); // list that stores all the file names of this directory
  if ((dp=opendir(countryDir))== NULL ) {	// open the directory
		perror("opendir"); exit(-1);
	}
  while ((direntp = readdir(dp)) != NULL ){ // for every file entity of the directory
    if (direntp->d_ino == 0 ) continue;
    int flag = strcmp(direntp->d_name,".") && strcmp(direntp->d_name,"..");
    if(flag!=0){
      // create the path of the file
      char *filePath=(char *)malloc(strlen(countryDir)+strlen(direntp->d_name)+2);
  		strcpy(filePath,countryDir);
			if(filePath[strlen(filePath)-1]!='/')
  			strcat(filePath,"/");
  		strcat(filePath,direntp->d_name);
      struct stat 	st;
      if(stat(filePath,&st) == -1){ perror("stat: "); exit(-1); }
      if(!S_ISREG(st.st_mode)){ // if the file is not a regular file, skip it
        printf("File: %s is not a regular file, so it is skipped\n",direntp->d_name);
        continue;
      }
      (*filesSeen) = insertString((*filesSeen),filePath); // add to the file list that stores all the files that we have seen

      filesOfDir = insertString(filesOfDir,filePath); // add every file of the directory to a list
      fileCount++;  // increase the file counter
      free(filePath);
    }
  }
  closedir(dp); // finally close the directory

  pthread_mutex_lock(&totalFilesMutex);
    totalFiles+=fileCount;  // increase the total files that the threads have to read
  pthread_mutex_unlock(&totalFilesMutex);

  producer(filesOfDir); // call producer in order to add every file name of this directory to the cyclic buffer

  deleteStringList(filesOfDir); // delete the temporary list
}

void readNewFilesOfDir(char *countryDir,Data database,stringList *filesSeen,int *foundNewFile){
  // search for new files in a directory, the old files that we have already read are stored in the filesSeen list
  DIR 	*dp;
	struct 	dirent *direntp;
  stringList filesOfDir = initializeStringList(); // list that stores all the file names of this directory
  int fileCount=0;  // counts how many files the directory has
  if ((dp=opendir(countryDir))== NULL ) {	// open the directory
		perror("opendir"); exit(-1);
	}
  while ((direntp = readdir(dp)) != NULL ){ // for every file of the directory
    if (direntp->d_ino == 0 ) continue;
    int flag = strcmp(direntp->d_name,".") && strcmp(direntp->d_name,"..");
    if(flag!=0){
      // create the path of the file
      char *filePath=(char *)malloc(strlen(countryDir)+strlen(direntp->d_name)+2);
  		strcpy(filePath,countryDir);
			if(filePath[strlen(filePath)-1]!='/')
  			strcat(filePath,"/");
  		strcat(filePath,direntp->d_name);
      struct stat 	st;
      if(stat(filePath,&st) == -1){ perror("stat: "); exit(-1); }
      if(!S_ISREG(st.st_mode)){ //if the file is not a a regular file, skip it
        printf("File: %s is not a regular file, so it is skipped\n",direntp->d_name);
        continue;
      }
      if(!stringExists((*filesSeen),filePath)){ // if the file path does not exist in the filesSeen list
        // then it is a new file and should be read
        printf("Found new file: %s\n",filePath);
        (*foundNewFile) = 1;
        (*filesSeen) = insertString((*filesSeen),filePath); // insert the file to the filesSeen list
        filesOfDir = insertString(filesOfDir,filePath); // insert the file to the temporary list
        fileCount++;  // increase the file counter
      }
      free(filePath);
    }
  }
  closedir(dp); // finally close the directory
  if(fileCount==0){ // if no new files were found
    return;
  }
  pthread_mutex_lock(&totalFilesMutex);
    totalFiles+=fileCount;  // increase the total files that the threads have to read
  pthread_mutex_unlock(&totalFilesMutex);

  producer(filesOfDir); // call producer in order to add every new file name of this directory to the cyclic buffer

  deleteStringList(filesOfDir); // delete the temporary list
}
