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
