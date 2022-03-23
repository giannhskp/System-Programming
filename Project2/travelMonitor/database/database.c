#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
// #include "bloomFilter.h"
// #include "hashTable.h"
// #include "countryLists.h"
// #include "skipList.h"
// #include "errorChecking.h"
// #include "pipeReadWrite.h"
#include "../bloomFilter/bloomFilter.h"
#include "../linkedLists/countryLists.h"
#include "../skipList/skipList.h"
#include "../errorChecking/errorChecking.h"
#include "../pipeReadWrite.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>


extern int bloomSize; // size of the bloom filter that the user have given (see main.c)
int hashSize=1000; // starting size of the hash table
// the hash table is resizable so the size is increasing when more records are inserted

int counterCheck=1;

// node that keeps all the info for every virus (virusNode)
typedef struct vn{
  char *virus;  // the name of the virus (only stored here)
  bloomFilter bloom;  // a bloom filter for the virus
  skipList vaccinated_persons;  // the vaccinated_persons for the virus
  skipList not_vaccinated_persons;  // the not_vaccinated_persons for the virus
  struct vn *nextVirus; // pointer to the next virus (list of virusNodes)
}virusNode;
typedef virusNode *VirusList;

typedef struct data{
  VirusList viruslist;  // list of virusNodes
  // every node of the list corresponds to a virus, and contains all tha databases for this virus (bloomFilter and skipLists)
  HashTable ht; // hash table that contains the info of each citizen (citizenId,firstName,lastName,age,country)
  // hash table is used in order to avoid data duplication, as a citizen may exist in many skipList (of different viruses)
  // therefore we only store the citizen data on the hashTable and have a pointer to it at the skipLists
  countryList countrylist;  // list of countries that cointains all the countries that the citizens in the hashTable have
  // this list is used in order to avoid data duplication in country names
  // every country name is stored in memmory only one time (at the country list)
  // every citizen inserted to the hashTable has a pointer to that country name, instead of "saving" for every citizen it's country
}DataNode;
typedef DataNode *Data; // all the data can be accessed from this pointer
// so in the main programm (main.c) we only have one "variable" of type Data, that stores all our data
// every time a function is called this variable is passed as argument in order to access all the data

void sendBloomFilters(int fd, Data database){ // sends every bloomFilter of the database to the travelMonitor
  VirusList virusL=database->viruslist;
  if(virusL==NULL){
    return;
  }
  while(virusL!=NULL){  // for every virus in the virusList
    sendMessage(fd,virusL->virus);  // first send the virusName
    sendBloom(fd,virusL->bloom);  // and after send the bloomFilter
    virusL=virusL->nextVirus; // go to the next virus
  }
  // after sending all the bloomFilters, send a message in order sto show to the travelMonitor that all the bloomFilters were sent
  char tempBuff[]="ENDOFBLOOMFILTERS";
  sendMessage(fd,tempBuff);
}

VirusList createVirusNode(char *virus){ // create/initialize a virusNode
  VirusList virusnode=malloc(sizeof(virusNode));  //create the node
  virusnode->virus=malloc(sizeof(char)*(strlen(virus)+1));  // allocate memmory in order to save the name of the virus
  strcpy(virusnode->virus,virus);
  virusnode->bloom=createBloom(bloomSize);  // create/initialize the bloom filter for this virus
  virusnode->vaccinated_persons=initializeSL(); // initialize vaccinated_persons skipList
  virusnode->not_vaccinated_persons=initializeSL(); // initialize not_vaccinated_persons skipList
  virusnode->nextVirus=NULL;  // set next as NULL because the new virusNode will be inserted at the end of the viruslist
  return virusnode;
}

void deleteVirusNode(VirusList virusnode){  //delete/free virusnode
  free(virusnode->virus);
  // for the bloomFilter and the skipLists we use the funcrions that are implemented and explained in the corresponding .c files
  deleteBloom(virusnode->bloom);
  destroySL(virusnode->vaccinated_persons);
  destroySL(virusnode->vaccinated_persons);
  free(virusnode);
  return;
}

Data initializeData(){  // initialize all the data (at the start of the program)
  Data d=malloc(sizeof(DataNode));
  d->ht=htCreate(hashSize); // initialize hashTable
  d->viruslist=NULL;  // virus list is empty
  d->countrylist=initializeCountryList(); // initialize the country list
  return d;
}

void deleteData(Data database){ // delete/free all the data (at the end of the program)
  htDestroy(database->ht);
  deleteCountryList(database->countrylist);
  VirusList virusL=database->viruslist;
  VirusList next=NULL;
  while(virusL!=NULL){  // for every virusNode in the VirusList
    free(virusL->virus);
    deleteBloom(virusL->bloom);
    destroySL(virusL->vaccinated_persons);
    destroySL(virusL->not_vaccinated_persons);
    next=virusL->nextVirus;
    free(virusL);
    virusL=next;
  }
  free(database);
}

void insertFromFile(Data database,char *citizenID,char *firstName,char *lastName,char *country,int age,char *virusName,char *vaccinated,char *date){
  // this function is used in order to insert a record to the "database" from the inputFile
  // it is only called fron readCitizenRecordsFile function (see below)
  VirusList virusL=database->viruslist; // get the virusList
  VirusList prev=NULL;
  while(virusL!=NULL){  // search for the virusNode that corresponds to the given virus
    if(strcmp(virusL->virus,virusName)==0){ // if the virusNode is found
      break;
    }else{
      prev=virusL;
      virusL=virusL->nextVirus;
    }
  }
  if(virusL==NULL){ //virus does not exist
    virusL=createVirusNode(virusName);  // create a new virusNode for the given virus
    if(prev==NULL){ // if it is the first virusNode of the virusList
      database->viruslist=virusL;
    }else{
      prev->nextVirus=virusL;
    }
  }
  // virus exists or was just created
  char *getCountryPointer;  // get a pointer to the country "string" so as to save the pointer inside the hashTable node
  database->countrylist=insertGetCountry(database->countrylist,country,&getCountryPointer);
  // insert the citizen to the hash table. If the citizen already exists he will not be inserted again
  // in both cases the htSearchOrInsert return a pointer to the node that the citizen data are stored
  citizenRecord rec=htSearchOrInsert(database->ht,citizenID,firstName,lastName,getCountryPointer,age);
  // if htSearchOrInsert return NULL, that mean that a citizen with the same citizenId but different data already exists in the hash table
  if(rec==NULL){
    // in this case an error message is printed, and the record is not inserted in the "database" (bloomFilter or skipList)
    if(strcmp(vaccinated,"YES")==0){
      printf("INCONSISTENT RECORD (record with same id but different data is also in the file): %s %s %s %s %d %s %s %s\n",citizenID,firstName,lastName,country,age,virusName,vaccinated,date);
    }else{
      printf("INCONSISTENT RECORD (record with same id but different data is also in the file): %s %s %s %s %d %s %s\n",citizenID,firstName,lastName,country,age,virusName,vaccinated);
    }
    return;
  }
  if(strcmp(vaccinated,"YES")==0){  // if the record cointains YES in the vaccinated "section"
    if(searchSL(virusL->not_vaccinated_persons,citizenID)){
      // check if the citizen already exists in the not_vaccinated_persons skip list
      // in this case (ONLY WHEN INSERTING FROM FILE) the record is considered as vaccinated and error message is printed
      printf("INCONSISTENT RECORD (record with NO is also in the file): %s %s %s %s %d %s %s %s\n",citizenID,firstName,lastName,country,age,virusName,vaccinated,date);
    }else{   //the citizen does not exist in the not_vaccinated_persons skip list of this virus
      bloomInsert(virusL->bloom,citizenID);   // insert the citizen to the bloom filter of the given virus
      int inserted=insertSL(virusL->vaccinated_persons,getId(rec),date,rec); // insert him to the vaccinated_persons skipList of this virus
      if(!inserted) // if the record is not inserted, that mean that it already exists in the vaccinated_persons skipList, so error message is printed (inconsistent record)
        printf("INCONSISTENT RECORD (already recorded as vaccinated): %s %s %s %s %d %s %s %s\n",citizenID,firstName,lastName,country,age,virusName,vaccinated,date);
      // else
      //   printf("inserted: %d\n",counterCheck++);
    }
  }else{  // the record contains NO in vaccinated "section" (it is checked at readCitizenRecordsFile function that record surely cointains YES or NO, see below)
    if(searchSL(virusL->vaccinated_persons,citizenID)){
      // check if the citizen already exists in the vaccinated_persons skip list
      // in this case (ONLY WHEN INSERTING FROM FILE) the record is considered as vaccinated and error message is printed
      printf("INCONSISTENT RECORD (record with YES is also in the file): %s %s %s %s %d %s %s\n",citizenID,firstName,lastName,country,age,virusName,vaccinated);
    }else{  //the citizen does not exist in the vaccinated_persons skip list of this virus
      // so insert the citizen to the not_vaccinated_persons skipList of the virus
      int inserted=insertSL(virusL->not_vaccinated_persons,getId(rec),NULL,rec);
      if(!inserted) // if the record is not inserted, that mean that it already exists in the not_vaccinated_persons skipList, so error message is printed (inconsistent record)
        printf("INCONSISTENT RECORD (already recorded as NOT vaccinated): %s %s %s %s %d %s %s\n",citizenID,firstName,lastName,country,age,virusName,vaccinated);
      // else
      //   printf("inserted: %d\n",counterCheck++);
    }
  }
}

void readCitizenRecordsFile(Data database,char *fileName){
  FILE *fp=fopen(fileName,"r");
  if(fp==NULL){
    perror("Error");
    return;
  }
  while(!feof(fp)){ // for every line of the file (every record)
    char buffer[1000];
    char citizenId[50];
    char firstName[50];
    char lastName[50];
    char country[50];
    int age;
    char virusName[50];
    char vaccinated[50];
    char date[50];
    if(fscanf(fp,"%[^\n]\n",buffer)<0){ //read the line
      continue;
    }
    char extra[50];
    int read=sscanf(buffer,"%s %s %s %s %d %s %s %s %s",citizenId,firstName,lastName,country,&(age),virusName,vaccinated,date,extra);
    // split the line in the corresponding "fields"
    if(strcmp(vaccinated,"NO")==0 && read==8){
      // case that the record contains NO in the vaccinated "field" and also have date
      printf("ERROR IN RECORD: %s %s %s %s %d %s %s %s\n",citizenId,firstName,lastName,country,age,virusName,vaccinated,date);
    }else if(strcmp(vaccinated,"YES")==0 && read==7){
      // case that the record contains YES in the vaccinated "field" but dont have date
      printf("ERROR IN RECORD: %s %s %s %s %d %s %s\n",citizenId,firstName,lastName,country,age,virusName,vaccinated);
    }else if(read!=8 && read!=7){
      // a record can only have 7 or 8 fields, so print error message
      printf("ERROR IN RECORD %s\n",buffer);
    }else{

      if(strcmp(vaccinated,"NO")==0){ // correct NO record, so insert it
        // no records have no date, so we pass NULL at the date field
        if(validCountryOrName(firstName) && validCountryOrName(lastName) && validCountryOrName(country) && validAge(age) && validVirus(virusName))
          insertFromFile(database,citizenId,firstName,lastName,country,age,virusName,vaccinated,NULL);
        else
          printf("ERROR IN RECORD %s\n",buffer);
      }else if(strcmp(vaccinated,"YES")==0){  // correct YES record, so inset it
        // if(!validDate(date))
        //   printf("ERROR IN THE DATE OF RECORD %s\n",buffer);
        // else
        if(validCountryOrName(firstName) && validCountryOrName(lastName) && validCountryOrName(country) && validAge(age) && validVirus(virusName) && validDate(date))
          insertFromFile(database,citizenId,firstName,lastName,country,age,virusName,vaccinated,date);
        else
          printf("ERROR IN RECORD %s\n",buffer);
      }else{
        // if the record does neither cointain YES nor NO at vaccinated "field"
        printf("ERROR IN RECORD %s\n",buffer);
      }
    }
  }
  // after reading all the lines of the file, close it
  fclose(fp);
}


char *vaccineStatus(Data database,char *citizenID,char *virusName){
  // this function checks if the citizen with citizenID is vaccinated for the virusName
  // if he is vaccinated, the vaccination date is returned
  VirusList virusL=database->viruslist;
  while(virusL!=NULL){  //search the virusList in order to find the virusNode of the given virus
    if(strcmp(virusL->virus,virusName)==0){ // found the node
      break;
    }else{
      virusL=virusL->nextVirus;
    }
  }
  if(virusL==NULL){ // the virus does not exist in the database, so the citizen is surely not vaccinated
    return NULL;
  }else{  // if the virus exists
    // search if the citizen exists in the vaccinated_persons skipList of the given virus, and if he exists get the date vaccinated
    char *date=searchGetDateSL(virusL->vaccinated_persons,citizenID);
    return date;
  }
}

void vaccineStatusAll(Data database,char *citizenID,int fd){
  // this function is used to implement /searchVaccinationStatus query
  citizenRecord citizen=htSearch(database->ht,citizenID); // search if the citizen with the given id exists in the database
  if(citizen==NULL){  // if the citizen does not exist send a message that indicates that the query is done
    char tempBuff4[]="ENDOF/SEARCHVACCINATIONSTATUS";
    sendMessage(fd,tempBuff4);  // send mesasge to the travelMonitor
    return;
  }
  char tempBuff[200]; // create a string that contains the id,firstName,lastName and the country of the citizen
  sprintf(tempBuff,"%s %s %s %s",getId(citizen),getFirstName(citizen),getLastName(citizen),getCountry(citizen));
  sendMessage(fd,tempBuff); // send the message to the travelMonitor
  char tempBuff2[100];  // create a string that contains the age of the citizen
  sprintf(tempBuff2,"AGE %d",getAge(citizen));
  sendMessage(fd,tempBuff2);  // send it to the travelMonitor
  VirusList virusL=database->viruslist;
  if(virusL==NULL){
    printf("Empty Database\n");
  }
  while(virusL!=NULL){  // for every virus in the virusList
    // search if the citizen exists in the vaccinated_persons skipList of the given virus, and if he exists get the date vaccinated
    char *date=searchGetDateSL(virusL->vaccinated_persons,citizenID);
    if(date==NULL){ // if date is NULL, that means that the citizen does not exist in the vaccinated_persons skipList
      // then search if the citizen exists in the not_vaccinated_persons skipList of the given virus
      // if he exists, send to the travelMonitor the virus name and NOT YET VACCINATED (as he is recorded as not vaccinated for this virus)
      if(searchSL(virusL->not_vaccinated_persons,citizenID)){
        char tempBuff3[200];
        sprintf(tempBuff3,"%s NOT YET VACCINATED",virusL->virus);
        sendMessage(fd,tempBuff3);
      }
      // if he does not exist in any skipList of the virus, dont send anything for this virus
    }else{  //if date is NOT NULL, then the citizen is vaccinated
      char tempBuff3[200];  // so send to the travelMonitor a message with the virusName and the vaccination date
      sprintf(tempBuff3,"%s VACCINATED ON %s",virusL->virus,date);
      sendMessage(fd,tempBuff3);
    }
    virusL=virusL->nextVirus; // go to the next virus
  }
  // after sending all the info, send a message that indicates that the query is done
  char tempBuff4[]="ENDOF/SEARCHVACCINATIONSTATUS";
  sendMessage(fd,tempBuff4);
}
