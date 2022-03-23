#include "linkedLists/stringList.h"
#include "database/database.h"
stringList readDirs(char *);
void readFilesOfDir(char *,Data ,stringList *);
void readNewFilesOfDir(char *,Data ,stringList *,int *);
