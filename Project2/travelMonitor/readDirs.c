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
#include "linkedLists/stringList.h"
#include "database/database.h"

stringList readDirs(char *rootDir){
  // reads the original directory given by the user, and returns a list with the paths of the directory that are inside the original directory
  DIR 	*dp;
	struct 	dirent *direntp;
  stringList countries = initializeStringList();

  if ((dp=opendir(rootDir))== NULL ) {	// open the directory
		perror("opendir"); exit(-1);
	}
  while ((direntp = readdir(dp)) != NULL ){ // for every entity inside the directory
    if (direntp->d_ino == 0 ) continue;
    int flag = strcmp(direntp->d_name,".") && strcmp(direntp->d_name,"..");
    if(flag!=0){
      char *dirPath=(char *)malloc(strlen(rootDir)+strlen(direntp->d_name)+2);  // the path of the directory will be stored here
  		strcpy(dirPath,rootDir);  // copy the name of the original directory
			if(dirPath[strlen(dirPath)-1]!='/')
  			strcat(dirPath,"/");
  		strcat(dirPath,direntp->d_name);  // concat the name of the country directory
      struct stat 	st;
      if(stat(dirPath,&st) == -1){ perror("statq: "); exit(-1); }
      if(!S_ISDIR(st.st_mode)){ //if the file is not a directory, skip it
        continue;
      }
      countries = insertString(countries,dirPath);  // insert the directory path into the list
      free(dirPath);
    }
  }
  closedir(dp); // close the directory
  return countries; // return the list of the country/directory paths
}

void readFilesOfDir(char *countryDir,Data database,stringList *filesSeen){
  // read the files inside of a country directory, and insert the records of each file into the database
  DIR 	*dp;
	struct 	dirent *direntp;

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
      // read the file and insert every record of the file to the database
      readCitizenRecordsFile(database,filePath);  // readCitizenRecordsFile is implemented in database.c file and is the same one that was used in project1
      free(filePath);
    }
  }
  closedir(dp); // finally close the directory
}

void readNewFilesOfDir(char *countryDir,Data database,stringList *filesSeen,int *foundNewFile){
  // search for new files in a directory, the old files that we have already read are stored in the filesSeen list
  DIR 	*dp;
	struct 	dirent *direntp;

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
        printf("Reading new file: %s\n",filePath);
        (*foundNewFile) = 1;
        (*filesSeen) = insertString((*filesSeen),filePath); // insert the file to the filesSeen list
        readCitizenRecordsFile(database,filePath);  // read the records of the file and insert them to the database
      }
      free(filePath);
    }
  }
  closedir(dp); // finally close the directory
}
