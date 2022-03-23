#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// a list that stores info fore every request that the travelMonitor received
// for every request (every node of the list) it is stored:
//     the virusName, the travelDate, the countryFrom and if it was accepted or not

typedef struct reqn{  // every node of the list (every request)
  char *virus;
  char *date;
  char *country;
  int accepted;
  struct reqn *next;
}reqListNode;
typedef reqListNode *requestList;

requestList createRequestNode(char virusName[],char date[],char country[],int accepted){ // create a node of the list for the given request
  requestList newnode=(requestList)malloc(sizeof(reqListNode)); // create the node
  newnode->virus=(char *)malloc(sizeof(char)*(strlen(virusName)+1));
  strcpy(newnode->virus,virusName); // store the virusName
  newnode->date=(char *)malloc(sizeof(char)*(strlen(date)+1));
  strcpy(newnode->date,date); // store the travelDate
  newnode->country=(char *)malloc(sizeof(char)*(strlen(country)+1));
  strcpy(newnode->country,country); // store the country name
  newnode->accepted=accepted;
  newnode->next=NULL;
  return newnode;
}

requestList initializeRequestList(){  // initialize list
  return NULL;
}

requestList insertRequestToList(requestList list,char virusName[],char date[],char country[],int accepted){
  // inserts a new request to the list, every new request is inserted at the start of the list
  // we may have duplicate requests
  if(list==NULL){ // if list is emtry
    requestList newnode=createRequestNode(virusName,date,country,accepted);
    return newnode;
  }else{
    requestList newnode=createRequestNode(virusName,date,country,accepted); //create a new node for this country
    newnode->next=list; // attach it to the old start of the list
    return newnode;
  }
}


int insideDates(char *,char *,char *);

void getRequestStats(requestList list,char *virus,char *date1,char *date2,char *country){
  // this function is uses in order to print the stats of /travelStats
  int totalCounter=0;
  int acceptedCounter=0;
  int rejectedCounter=0;
  if(list==NULL){ // if list is emtry
    printf("No request were recieved\n");
    return;
  }else{
    while(list!=NULL){  //traverse the list, for every request that the travelMonitor have received
      if(strcmp(list->virus,virus)==0){ // is the request is for the given virus
        if( country==NULL || (strcmp(list->country,country)==0) ){
          // if country was not given, or if the country of the request is the same as the given one
          if(insideDates(date1,date2,list->date)){  // check if the travelDate is inside the 2 given dates
            totalCounter+=1;  // increase the totalCounter
            if(list->accepted)  // if this request was accepted
              acceptedCounter+=1;
            else  // if this request was rejected
              rejectedCounter+=1;
          }
        }

      }
      list=list->next;
    }
    // finally print the stats
    printf("TOTAL REQUESTS: %d\n",totalCounter);
    printf("ACCEPTED %d\n",acceptedCounter);
    printf("REJECTED %d\n",rejectedCounter);
    return;
  }
}


void deleteRequestList(requestList list){   // delete (free) the list
  requestList next;
  while(list!=NULL){
    next=list->next;
    free(list->date);
    free(list->virus);
    free(list->country);
    free(list);
    list=next;
  }
}
/////////////////////////////////


int smallerDate(char *year1,char *month1,char *day1,char *year2,char *month2,char *day2){ // compare two given dates
  int y1,m1,d1,y2,m2,d2;
  y1=atoi(year1);
  m1=atoi(month1);
  d1=atoi(day1);
  y2=atoi(year2);
  m2=atoi(month2);
  d2=atoi(day2);
  if(y1<y2){
    return 1;
  }else if(y1==y2 && m1<m2){
    return 1;
  }else if (y1==y2 && m1==m2 && d1<=d2){
    return 1;
  }
  return 0;
}

int insideDates(char *date1,char *date2,char *compareDate){
  // returns 1 if: date1 <= compareDate <= date2 and 0 otherwise
  char day1[10];
  char month1[10];
  char year1[10];
  sscanf(date1,"%[^-]-%[^-]-%[^-]-",day1,month1,year1); // convert the date from 1 string to 3 ints (day,month,year)
  char day2[10];
  char month2[10];
  char year2[10];
  sscanf(date2,"%[^-]-%[^-]-%[^-]-",day2,month2,year2);
  char compareDay[10];
  char compareMonth[10];
  char compareYear[10];
  sscanf(compareDate,"%[^-]-%[^-]-%[^-]-",compareDay,compareMonth,compareYear);

  if(smallerDate(year1,month1,day1,compareYear,compareMonth,compareDay) && smallerDate(compareYear,compareMonth,compareDay,year2,month2,day2)){
   return 1;
 }else{
   return 0;
 }
}
