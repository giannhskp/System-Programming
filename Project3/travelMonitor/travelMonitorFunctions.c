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
#include "bloomFilter/bloomFilter.h"
#include "readDirs.h"
#include "database/database.h"
#include "linkedLists/bloomList.h"
#include "socketReadWrite.h"

# define READ 0
# define WRITE 1

extern int numMonitors; // the number of monitors that will be created (given by the users)

int findMonitorByCountry(char country[], stringList countriesOfMonitors[]){ // find in which monitor a country is assigned
  for(int i=0;i<numMonitors;i++){
    if(stringExists(countriesOfMonitors[i],country)){
      return i;
    }
  }
  return -1;
}

int inside6Months(char vaccinationDate[],char travelDate[]){  // check if the vaccinationDate is at most 6 moths ago from travelDate
  char day[10];
  char month[10];
  char year[10];
  sscanf(vaccinationDate,"%[^-]-%[^-]-%[^-]-",day,month,year); // convert the vaccinationDate from 1 string to 3 ints (day,month,year)
  int givenDay=atoi(day);
  int givenMonth=atoi(month);
  int givenYear=atoi(year);

  char day2[10];
  char month2[10];
  char year2[10];
  sscanf(travelDate,"%[^-]-%[^-]-%[^-]-",day2,month2,year2); // convert the travelDate from 1 string to 3 ints (day,month,year)
  int travelDay=atoi(day2);
  int travelMonth=atoi(month2);
  int travelYear=atoi(year2);
  // check if the travel date is before the vaccination date
  // int this case 0 should be returned
  if(givenYear>travelYear){
    return -1;
  }else if(givenYear==travelYear && givenMonth>travelMonth){
    return -1;
  }else if (givenYear==travelYear && givenMonth==travelMonth && givenDay>travelDay){
    return -1;
  }

  if(travelMonth>6){  // convert the travel date to 6 moths before
    travelMonth-=6;
  }else{
    travelMonth = 12 - (6-travelMonth);
    travelYear-=1;
  }

  int flag=1;
  // check if the 6 months before the travel date is bigger or smaller than the vaccination date
  if(givenYear<travelYear){
    flag=0;
  }else if(givenYear==travelYear && givenMonth<travelMonth){
    flag=0;
  }else if (givenYear==travelYear && givenMonth==travelMonth && givenDay<travelDay){
    flag=0;
  }
  return flag;
}

int travelRequest(char citizenId[],char date[],char countryFrom[],char countryTo[],char virusName[], int fdboard[],bloomList bloomFiltersOfMonitors[], stringList countriesOfMonitors[]){
  int monitor=findMonitorByCountry(countryFrom,countriesOfMonitors);  // find to which monitor the given country is assigned
  if(monitor==-1){ printf("Country does not exist in the travelMonitor\n"); return -1; }
  bloomFilter bloomOfVirus=getBloomFromList(bloomFiltersOfMonitors[monitor],virusName);  // get the bloomFilters of the monitor
  if(bloomOfVirus==NULL){ // if there is no bloom filter for this virus, then surely the citizen is not vaccinated
    printf("REQUEST REJECTED - YOU ARE NOT VACCINATED\n");
    return 0;
  }
  int exists=bloomExists(bloomOfVirus,citizenId); // check if the citizenId exists in the bloomFilter
  if(exists){ // if it exist "ask" the monitor if the citizen is vaccinated
    char tempBuff[]="/travelRequest";
    sendMessage(fdboard[monitor],tempBuff); // send the /travelRequest command to the monitor
    sendMessage(fdboard[monitor],citizenId);  // send the citizenId to the monitor
    sendMessage(fdboard[monitor],virusName);  // send the virusName to the monitor
    char *response=readMessage(fdboard[monitor]);  // read the response from the monitor
    if(strcmp(response,"YES")==0){  // if the monitor responded YES
      // if the monitor responded YES that means that the citizen is vaccinated
      char *vaccinationDate=readMessage(fdboard[monitor]); // read the vaccination date of the citizen
      int isInside6Months=inside6Months(vaccinationDate,date);
      if(isInside6Months==1){  // check if the vaccination date is inside 6 moths from the travel date
        printf("REQUEST ACCEPTED - HAPPY TRAVELS\n"); // if it is then the request is accepted
        free(vaccinationDate);
        free(response);
        char tempBuff2[]="ACCEPTED";
        sendMessage(fdboard[monitor],tempBuff2);  // inform the monitor that the request was accepted
        return 1;
      }else{  // if the date is not inside 6 months from the travel date
        if(isInside6Months==0)
          printf("REQUEST REJECTED - YOU WILL NEED ANOTHER VACCINATION BEFORE TRAVEL DATE\n");  // then the request is rejected
        else  // if the travel date is before the vaccination date
          printf("REQUEST REJECTED - YOU ARE NOT VACCINATED\n");  // then the request is rejected
        free(vaccinationDate);
        free(response);
        char tempBuff2[]="REJECTED";
        sendMessage(fdboard[monitor],tempBuff2);  // inform the monitor that the request was rejected
        return 0;
      }
    }else{  // if the monitor responded NO, then the citizen is not vaccinated
      printf("REQUEST REJECTED - YOU ARE NOT VACCINATED\n");
      free(response);
      return 0;
    }
  }else{  // if the citizenId does not exists to the bloomFilter then surely his is not vaccineted
    // so we dont need to "ask" the monitor
    printf("REQUEST REJECTED - YOU ARE NOT VACCINATED\n");
    return 0;
  }
}

void addVaccinationRecords(char country[],int pidArray[],int fdboard[],bloomList (*bloomFiltersOfMonitors)[], stringList countriesOfMonitors[]){
  int monitor=findMonitorByCountry(country,countriesOfMonitors); // find to which monitor the given country is assigned
  if(monitor==-1){ printf("Country does not exist in the travelMonitor\n"); return; }
  char tempBuff[]="/addVaccinationRecords";
  sendMessage(fdboard[monitor],tempBuff); // send the command to every monitor
  sendMessage(fdboard[monitor],country);  // also send the country name that a new file was added

  int bloomFilterSent=0;
  while(1){ // start reading the updated bloomFilters of the monitor
    char *virusName=readMessage(fdboard[monitor]);
    if(strcmp(virusName,"ENDOFBLOOMFILTERS")==0){ // that means that the monitor sent all the bloomFilter
      // if the monitor did not find a new file, it just sends this message, so we keep the old bloomFilter as they dont have changed
      free(virusName);
      break;
    }else{
      if(bloomFilterSent == 0){ // if the monitor found a new file, and it starts sending the bloomFilters
        deleteBloomList((*bloomFiltersOfMonitors)[monitor]);  // delete the old bloom filters of this monitor, as he will send the updated ones
        (*bloomFiltersOfMonitors)[monitor]=initializeBloomList();
        bloomFilterSent=1;
      }
      bloomFilter bf = readBloom(fdboard[monitor]);
      (*bloomFiltersOfMonitors)[monitor] = insertBloomToList((*bloomFiltersOfMonitors)[monitor],virusName,bf);
    }
  }
}

void searchVaccinationStatus(char citizenId[], int fdboard[]){
  for(int i=0;i<numMonitors;i++){
    char tempBuff[]="/searchVaccinationStatus";
    sendMessage(fdboard[i],tempBuff); // send the command to every monitor
    sendMessage(fdboard[i],citizenId);  // also send the id of the citizen
  }

  int ended[numMonitors]; // flag board that contains if the reading from every socket if finished
  struct pollfd pfds[numMonitors];  // use poll to monitor all the sockets
  // each place of the poll board corresponds to the socket of the equivelent children
  for(int i=0;i<numMonitors;i++){
    ended[i]=0;   // initialize the end board with zero as we have not started reading yet
    pfds[i].fd=fdboard[i];  // add the socket descriptor of the equivelent socket to the poll board
    pfds[i].events=POLLIN;      // we are only intersted in reading so we add only POLLIN in the inrested events of poll
  }
  int flag=1;
  while(flag){
    int retval = poll(pfds,numMonitors,0);
    flag=1;
    if(retval<0){ //if poll is interrupted by a signal, continue and redo poll
        flag=1;
        continue;
    }
    for(int i=0;i<numMonitors;i++){
      if((pfds[i].revents&POLLIN)){ // if something is written in this socket
          char *message=readMessage(fdboard[i]); // read the message from this monitor
          if(strcmp(message,"ENDOF/SEARCHVACCINATIONSTATUS")==0){ // if the monitor indicated that it has ended the query
            free(message);
            ended[i]=1;
          }else{
            printf("%s\n",message); // print the info that the monitor sent
            free(message);
          }
      }
    }
    flag=0;
    for(int i=0;i<numMonitors;i++){   //check if all sockets have ended reading
      if(ended[i]==0)
        flag=1;   // if we have not finished reading from at least one socket, loop until one socket is ready for reading
      // if we have finished reading from all the sockets the flag is 0 so the loop (and the reading) is stopped
    }
  }
}

void terminate(int fdboard[]){
  for(int i=0;i<numMonitors;i++){
    char tempBuff[]="/exit";
    sendMessage(fdboard[i],tempBuff); // send the command through the socket to every monitor in order to terminate
  }
}
