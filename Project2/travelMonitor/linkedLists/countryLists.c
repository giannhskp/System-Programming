#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


///////////////////////////
// This list contains all the countries that are inserted to the "database"
// the logic is that the string of every country is only stored here
// Therefore, every record from this country (in the hashTable) has a pointer to this string
// As a result every country is always stored in memmory only one time (no duplicate info)
typedef struct cn{
  char *country;
  struct cn *next;
}countryNode;
typedef countryNode *countryList;

countryList createCountryNode(char *country){ // create a node of the list for the given country
  countryList newnode=(countryList)malloc(sizeof(countryNode));
  newnode->country=(char *)malloc(sizeof(char)*(strlen(country)+1));
  strcpy(newnode->country,country);
  newnode->next=NULL;
  return newnode;
}

countryList initializeCountryList(){  // initialize list
  return NULL;
}

countryList insertGetCountry(countryList list,char *country,char **returnCountry){
  // this function searches the given country, if it exists returns a pointer to the string of the country
  // if it does not exist, a new node is created, the new country is inserted to the list and a pointer to the string is returned
  if(list==NULL){ // if list is emtry
    countryList newnode=createCountryNode(country);
    (*returnCountry)=newnode->country;
    return newnode;
  }else{
    countryList prev=NULL;
    countryList origin=list;
    while(list!=NULL){  //traverse the list
      if(strcmp(list->country,country)==0){ // if country is found
        (*returnCountry)=list->country; //return a pointer to the string
        return origin;
      }
      prev=list;
      list=list->next;
    }
    // if country does not exist
    countryList newnode=createCountryNode(country); //create a new node for this country
    (*returnCountry)=newnode->country;  //return a pointer to the string
    prev->next=newnode; // attach it to the end of the list
    return origin;
  }
}


void deleteCountryList(countryList list){   // delete (free) the list
  countryList next;
  while(list!=NULL){
    next=list->next;
    free(list->country);
    free(list);
    list=next;
  }
}
/////////////////////////////////

///////////////////////////
// list used for the function /populationStatus
// only used in the function populationStatus of the file database.c
typedef struct cl{
  char *country;
  int meetsConditionsCount;
  int totalCounter;
  struct cl *next;
}countryListNode;
//every node corresponds to a country and contains the name of the country, how many people exist in the records for a certain virus (YES & NO) (totalCounter)
// and also contains how many of them meet the conditions (have vacinated inside the given dates, if dates are given) (meetsConditionsCount)
typedef countryListNode *countryCounterList;

countryCounterList createCCLNode(char *country,int meetsConditions){  //create and initialize a node of the list
  countryCounterList newnode=(countryCounterList)malloc(sizeof(countryListNode));
  newnode->country=(char *)malloc(sizeof(char)*(strlen(country)+1));
  strcpy(newnode->country,country);
  newnode->meetsConditionsCount=meetsConditions;  // meetsConditions corresponds to if the person that we are "inserting" to the node meets the conditions (and must increase the counter)
  newnode->totalCounter=1;
  newnode->next=NULL;
  return newnode;
}

countryCounterList initializeCCL(){   // initialize the list
  return NULL;
}

countryCounterList increaseCounter(countryCounterList list,char *country,int meetsConditions){  // "insert" a new person to the list
  // we must increase the stats of the node that corresponds to the given country
  if(list==NULL){ // fi list is empty
    return createCCLNode(country,meetsConditions);
  }else{
    countryCounterList prev=NULL;
    countryCounterList origin=list;
    while(list!=NULL){
      if(strcmp(list->country,country)==0){   // find the node of the list that corresponds to the given country
        list->meetsConditionsCount+=meetsConditions;  // if the person meets the conditions, increase the counter
        list->totalCounter++;   // in every case increase the total counter
        return origin;
      }
      prev=list;
      list=list->next;
    }
    // if the country does not exist in the list
    prev->next=createCCLNode(country,meetsConditions);
    return origin;
  }
}

void printCCL(countryCounterList list){ // print the list
  // print every country, the total counter and the percentage of the people that meet the conditions
  while(list!=NULL){
    printf("%s : %d %.2f%%\n",list->country,list->meetsConditionsCount, ((double)list->meetsConditionsCount/(double)list->totalCounter)*100);
    list=list->next;
  }
}


void deleteCCL(countryCounterList list){  // delete (free) the list
  countryCounterList next;
  while(list!=NULL){
    next=list->next;
    free(list->country);
    free(list);
    list=next;
  }
}
