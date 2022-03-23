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
/////////////////////////////////
///////////////////////////
// list used for the function /popStatusByAge
// only used in the function popStatusByAge of the file database.c
// NOTE: the implementation and the logic behind this list is the same as above
// the only difference is that every time a person meets the conditions (i.e. vaccinated inside [date1,date2]) we increase
// a counter depending on his age (see ageListNode below)
typedef struct al{
  char *country;
  int below20;
  int totalBelow20;
  int below40;
  int totalBelow40;
  int below60;
  int totalBelow60;
  int over60;
  int totalOver60;
  struct al *next;
}ageListNode;
typedef ageListNode *countryAgeList;

countryAgeList createCALNode(char *country,int age,int meetsConditions){
  countryAgeList newnode=(countryAgeList)malloc(sizeof(ageListNode));
  newnode->country=(char *)malloc(sizeof(char)*(strlen(country)+1));
  strcpy(newnode->country,country);
  newnode->below20=0;
  newnode->below40=0;
  newnode->below60=0;
  newnode->over60=0;
  newnode->totalBelow20=0;
  newnode->totalBelow40=0;
  newnode->totalBelow60=0;
  newnode->totalOver60=0;
  if(age<20){
    (newnode->below20)+=meetsConditions;
    (newnode->totalBelow20)=1;
  }else if(age<40){
    (newnode->below40)+=meetsConditions;
    (newnode->totalBelow40)=1;
  }else if(age<60){
    (newnode->below60)+=meetsConditions;
    (newnode->totalBelow60)=1;
  }else{
    (newnode->over60)+=meetsConditions;
    (newnode->totalOver60)=1;
  }
  newnode->next=NULL;
  return newnode;
}

countryAgeList initializeCAL(){
  return NULL;
}

countryAgeList increaseAgeCounter(countryAgeList list,char *country,int age,int meetsConditions){
  if(list==NULL){
    return createCALNode(country,age,meetsConditions);
  }else{
    countryAgeList prev=NULL;
    countryAgeList origin=list;
    while(list!=NULL){
      if(strcmp(list->country,country)==0){
        if(age<20){
          (list->below20)+=meetsConditions;
          (list->totalBelow20)+=1;
        }else if(age<40){
          (list->below40)+=meetsConditions;
          (list->totalBelow40)+=1;
        }else if(age<60){
          (list->below60)+=meetsConditions;
          (list->totalBelow60)+=1;
        }else{
          (list->over60)+=meetsConditions;
          (list->totalOver60)+=1;
        }
        return origin;
      }
      prev=list;
      list=list->next;
    }
    prev->next=createCALNode(country,age,meetsConditions);
    return origin;
  }
}

void printCAL(countryAgeList list){
  while(list!=NULL){
    printf("%s :\n",list->country);
    if(list->totalBelow20!=0)
      printf("  0-20 %d %.2f%%\n",list->below20,((double)list->below20/(double)list->totalBelow20)*100);
    else
      printf("  0-20 %d 0.00%%\n",list->below20);
    if(list->totalBelow40!=0)
      printf("  20-40 %d %.2f%%\n",list->below40,((double)list->below40/(double)list->totalBelow40)*100);
    else
      printf("  20-40 %d 0.00%%\n",list->below40);
    if(list->totalBelow60!=0)
      printf("  40-60 %d %.2f%%\n",list->below60,((double)list->below60/(double)list->totalBelow60)*100);
    else
      printf("  40-60 %d 0.00%%\n",list->below60);
    if(list->totalOver60!=0)
      printf("  60+ %d %.2f%%\n",list->over60,((double)list->over60/(double)list->totalOver60)*100);
    else
      printf("  60+ %d 0.00%%\n",list->over60);
    list=list->next;
  }
}


void deleteCAL(countryAgeList list){
  countryAgeList next;
  while(list!=NULL){
    next=list->next;
    free(list->country);
    free(list);
    list=next;
  }
}
/////////////////////////////////
