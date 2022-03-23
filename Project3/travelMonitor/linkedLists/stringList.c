#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "stringList.h"


///////////////////////////
// this list is a sorted list of strings
// every node contains a string


stringList createStringNode(char *str){ // create a node of the list for the given string
  stringList newnode=(stringList)malloc(sizeof(node));
  newnode->string=(char *)malloc(sizeof(char)*(strlen(str)+1));
  strcpy(newnode->string,str);
  newnode->next=NULL;
  return newnode;
}

stringList initializeStringList(){  // initialize list
  return NULL;
}

// stringList insertGetString(stringList list,char *str,char **returnString){
//   // this function searches the given country, if it exists returns a pointer to the string of the country
//   // if it does not exist, a new node is created, the new country is inserted to the list and a pointer to the string is returned
//   if(list==NULL){ // if list is emtry
//     stringList newnode=createStringNode(str);
//     (*returnString)=newnode->string;
//     return newnode;
//   }else{
//     stringList prev=NULL;
//     stringList origin=list;
//     while(list!=NULL){  //traverse the list
//       if(strcmp(list->string,str)==0){ // if country is found
//         (*returnString)=list->string; //return a pointer to the string
//         return origin;
//       }
//       prev=list;
//       list=list->next;
//     }
//     // if country does not exist
//     stringList newnode=createStringNode(str); //create a new node for this country
//     (*returnString)=newnode->string;  //return a pointer to the string
//     prev->next=newnode; // attach it to the end of the list
//     return origin;
//   }
// }

stringList insertString(stringList list,char *str){
  // insert a string to the list, if the string already exists it is not inserted
  if(list==NULL){ // if list is emtry
    stringList newnode=createStringNode(str);
    return newnode;
  }else{
    stringList prev=NULL;
    stringList origin=list;
    while(list!=NULL){  //traverse the list
      if(strcmp(list->string,str)==0){ // if the same string already exists in the list
        return origin;
      }
      if(strcmp(list->string,str)>0){ // find the "place" that the new string will be inserted in order to keep the list sorted
        stringList newnode=createStringNode(str); //create a new node for this string
        if(prev==NULL){ // if it should be inserted at the start of the list
          newnode->next=origin; // attach the previous first node to the new node
          return newnode; // return the new node as the start of  the list
        }else{  // if it should be inserted somewhere in the middle of the list
          stringList temp=prev->next;
          prev->next=newnode; // attach it to the previous node
          newnode->next=temp;
          return origin;
        }
      }
      prev=list;
      list=list->next;
    }
    // if it should be inserted at the end of the list
    stringList newnode=createStringNode(str); //create a new node for this country
    prev->next=newnode; // attach it to the end of the list
    return origin;
  }
}

int stringExists(stringList list,char *str){
  // this function searches the given string, if it exists returns 1 else it returns 0
  if(list==NULL){ // if list is empty
    return 0;
  }else{
    while(list!=NULL){  //traverse the list
      if(strcmp(list->string,str)==0){ // if country is found
        return 1;
      }
      list=list->next;
    }
    return 0;
  }
}


void deleteStringList(stringList list){   // delete (free) the list
  stringList next;
  while(list!=NULL){
    next=list->next;
    free(list->string);
    free(list);
    list=next;
  }
}
/////////////////////////////////
