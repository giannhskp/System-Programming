#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bloomList.h"


///////////////////////////
// this is a list of bloom filters
// every node of the list contains a bloom filter and the name of the virus that the bloom filter corresponds to


bloomList createBloomNode(char *virus,bloomFilter bf){ // create a node of the list for the given country
  bloomList newnode=(bloomList)malloc(sizeof(bfListNode));
  // the name of the virus and the bloom filter are dynamically allocated before inserted to the list
  // so we just save a pointer to them
  newnode->virus=virus;
  newnode->bf=bf;
  newnode->next=NULL;
  return newnode;
}

bloomList initializeBloomList(){  // initialize list
  return NULL;
}

bloomList insertBloomToList(bloomList list,char *virus,bloomFilter bf){
  // insert the given bloom filter of the given virus to the list
  if(list==NULL){ // if list is empty
    bloomList newnode=createBloomNode(virus,bf);
    return newnode;
  }else{
    bloomList prev=NULL;
    bloomList origin=list;
    while(list!=NULL){  //traverse the list
      if(strcmp(list->virus,virus)==0){ // if the virus (and it's bloomFilter) already exists
        return origin;  // dont insert the new one
      }
      prev=list;
      list=list->next;
    }
    // if the virus does not exist, insert the bloom filter at the end of the list
    bloomList newnode=createBloomNode(virus,bf); //create a new node for this virus
    prev->next=newnode; // attach it to the end of the list
    return origin;
  }
}

bloomFilter getBloomFromList(bloomList list,char *virus){
  // searches the list for the given virus, and if it exists it returns it's bloomFilter
  if(list==NULL){ // if list is emtry
    return NULL;
  }else{
    while(list!=NULL){  //traverse the list
      if(strcmp(list->virus,virus)==0){ // if virus is found
        return list->bf;  // return it's bloomFilter
      }
      list=list->next;
    }
    return NULL;
  }
}


void deleteBloomList(bloomList list){   // delete (free) the list
  bloomList next;
  while(list!=NULL){
    next=list->next;
    free(list->virus);
    deleteBloom(list->bf);
    free(list);
    list=next;
  }
}
/////////////////////////////////
