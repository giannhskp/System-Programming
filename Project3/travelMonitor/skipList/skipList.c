#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../hashTable/hashTable.h"
#include "../linkedLists/countryLists.h"

#define MAX_LEVEL 30  // the max level that tha skipList can reach (practically it almost never reaches that limit)

// the skip list constist of a list of levels (slLevelNode) that dynamicaly is getting bigger (as levels increase)
// every level contains a list of the keys that exist in this level that dynamicaly is getting bigger (as a new key is inserted on a level)
// so a skip list is a list of lists
typedef struct node{
  char *key;  // the key that this node corresponds to
  // NOTE: in every node a pointer to a sting is kept. The key is actually stored only one time (in the hashTable).
  // So if the same citizen exists in multiple skipLists, the key itself is not stored in memory multiple times (no data duplication)
  char *date; // the date that this person was vaccinated (in not_vaccinated_persons skipList date=NULL)
  // NOTE: similarely with the key is actually stored only in the bottom level, and every other level contains (NULL)
  // in this way tha skipList has no duplicate information about the records
  void *down; // a ponter to the corresponding node in the below level of the skipList
  // bottom level contains a pointer to the actual record that corresponds to the key, and
  //  all the other levels contain (slNode *) to the node with the same citizenId (key) from the below level
  struct node *next;  // pointer to the next node of the list
}slNode;  // the node of the lists in every level

typedef struct level{
  int level;  // stored the level
  slNode *list; // the list of nodes(keys) that exist to this level
  struct level *downLevel;  // pointer to the below level
}slLevelNode; // list of levels

typedef struct sl{
  int records;  // counter of how many "record" were inserted in the skipList
  int totalLevels;  // the total number of level that the skip list currently have
  slLevelNode *topLevel;  // pointer to the top (higher) level of the skipList
}slHead;  // skipList head node
typedef slHead *skipList;

// int getRecords(skipList sl){
//   return sl->records;
// }

skipList initializeSL(){  //create/initialize the skipList
  skipList sl=(skipList)malloc(sizeof(slHead)); //create the head node
  sl->records=0;
  sl->totalLevels=1;
  sl->topLevel= (slLevelNode *)malloc(sizeof(slLevelNode)); //create the bottom level
  sl->topLevel->level=1;  // NOTE: bottom level is counted as number 1
  sl->topLevel->list=NULL;  // level is empty
  sl->topLevel->downLevel=NULL; // there is no level below the bottom level
  return sl;
}

int flipCoin(){
  return rand()%2;
}

slNode *createNode(char *key,char *date,slNode * next,void *down,int level){  //create a node of the list
  slNode *newnode=malloc(sizeof(slNode)); //create the node
  newnode->next=next; // attach it to the given next node
  newnode->down=down; // bottom level contains pointer to record and other levels contain ponter to node of the below level
  newnode->key=key; // store a pointer to the key that is stored in the hashTable's node corresponding to this citizen (so as not to have data duplication on the key)
  if(date==NULL){
    newnode->date=NULL;
  }else{  // only bottom level contains the date
    newnode->date=malloc(sizeof(char)*(strlen(date)+1));
    strcpy(newnode->date,date);
  }
  return newnode;
}

slNode *insert(slLevelNode *levelNode,slNode *list,char *key,int *inserted,citizenRecord rec,char *date){   //recursive function for the insertion in the skip list
  // this function is actually only called by the function insertSL (see below)
  if(list==NULL){ // if the skipList is empty
    list=createNode(key,date,NULL,rec,1); //create a node for the bottom level
    levelNode->list=list; // attach it to the levelNode
    (*inserted)=1;  // set flag that the record was inserted
    if(flipCoin()){ // decide if the key will arise to the above level
      return list;  // if yes, return the node that was just created
      // in order the above node,to point to this node
    }else{
      return NULL;
    }
  }else{
    slNode *prev=NULL;
    while(list->next!=NULL){  // start traversing to the list of this level
      // we start traversing from the node that was given by the above level as argument (list)
      if(strcmp(list->next->key,key)<=0){ // compare the keys in order to stop traversing the list
        prev=list;
        list=list->next;
        continue;
      }
      break;
    }
    if(levelNode->level==1){  // if we are at level 1 (bottom level)
      if(strcmp(list->key,key)==0){ // if the key already exists
        (*inserted)=0;  // set flag to 0 as the new record will not be inserted
        return NULL;  // dont insert the record, and return NULL
      }else{  // the key does not exist, so we must insert it
        slNode *newnode;
        if(strcmp(list->key,key)>0){  // if the new key must be inserted at the start of the list
          newnode=createNode(key,date,list,(void *)rec,1);  //create the new node, and attach the old first node to the new node
          // also set the pointer of the new node to point to the record that corresponds to the key (rec)
          levelNode->list=newnode;  //attach the new node to the levelNode as it is now the first node of the list
        }else{
          // if it is not being inserted at the start of the list
          // create a new node and attach it to the previous and the next node of the list
          newnode=createNode(key,date,list->next,(void *)rec,1);
          list->next=newnode;
        }
        (*inserted)=1;  // set the flag
        if(flipCoin()){ //decide if the new node will be inserted to the above level
          // if yes, return the node to the above level (return from the recursive call)
          return newnode;
        }else{
          // else return NULL, and end the recursion
          return NULL;
        }
      }
    }else{  // if we are not to the bottom level
      // we must continue searching to the below node with the same key with the node that we stopped the traversing on this level
      // this is accomplished with a recursive call of the insert function (for the below level every time)
      // and by giving as argument the node from which it should start searching the below level
      slNode *newDownNode;
      if( prev==NULL && (strcmp(list->key,key)>0) ) {
        // if we must start seaching from the start of the list of the below level, give as parameter the first node of the below level list (levelNode->downLevel->list)
        newDownNode=insert(levelNode->downLevel,levelNode->downLevel->list,key,inserted,rec,date);
      }else{
        // if we must continue searching from the bellow node of the node that we stopped in this level, give as parameter the pointer to the below node (list->down)
        newDownNode=insert(levelNode->downLevel,list->down,key,inserted,rec,date);
      }
      // we have returned from the recursive call
      if(newDownNode==NULL){  // that means that the new node does not need to be inserted to this level
        return NULL;
      }else{
        // that means that the new node does must be inserted also in this level
        slNode *newnode;
        if(strcmp(list->key,key)>0){  // if the new key must be inserted at the start of the list
          newnode=createNode(key,NULL,list,newDownNode,levelNode->level);  //create the new node, and attach the old first node to the new node
          // also set the pointer of the new node to point to the node of the below level (newDownNode)
          levelNode->list=newnode;  //attach the new node to the levelNode as it is now the first node of the list
        }else{
          // if it is not being inserted at the start of the list
          // create a new node and attach it to the previous and the next node of the list
          newnode=createNode(key,NULL,list->next,newDownNode,levelNode->level);
          list->next=newnode;
        }
        if(flipCoin()){ //decide if the new node will also be inserted to the above level
          // if yes, return the node to the above level (return from the recursive call)
          return newnode;
        }else{
          // else return NULL, and end the recursion
          return NULL;
        }
      }
    }
  }
}

int insertSL(skipList head,char *key,char *date,citizenRecord rec){
  //this function is called from database.c file in order to insert a "record" to the given skipList
  int inserted=0; //flag that is return from insert function so as to know if the record was actually inserted
  slNode *newnode=insert(head->topLevel,head->topLevel->list,key,&inserted,rec,date); // insert the record to the skip list
  while(newnode!=NULL && head->totalLevels<MAX_LEVEL){  // if the new record have to be inserted to new up level
    // a new top level is created (height of the skip list is increased by one)
    // the new top level only contains this new record
    slLevelNode *newLevel=(slLevelNode *)malloc(sizeof(slLevelNode)); // create the new level
    newLevel->level=(head->topLevel->level)+1;
    newLevel->downLevel=head->topLevel; // attach the previous top level to the new top level
    newLevel->list=createNode(newnode->key,NULL,NULL,newnode,newLevel->level);  // create the key node that will be inserted to the level list
    head->topLevel=newLevel;  // assing the new level as the top level of the skipList
    head->totalLevels++;  //increase the total level counter
    if(flipCoin()){ // decide if the new key will arrise to one level more
      // case that the key does not create another new level (50% chance)
      newnode=NULL;
    }else{
      // case that the key arrises to one more level upwards (50% chance)
      newnode=newLevel->list;
    }
  }
  if(inserted){
    head->records++;
  }
  return inserted;
}

int search(slLevelNode *levelNode,slNode *list,char *key,char **date){
  // this recursive function in order to search if the given key exists in the skip list
  // this function is only called by searchSL function (see below)
  // if the key is found, the date is returned using the date argument
  // the function returns 1 if the key was found and 0 otherwise
  if(list==NULL){ // if skipList is empty
    return 0;
  }else{
    slNode *prev=NULL;
    while(list->next!=NULL){
      //traverse the list of the current level starting from the given node (list argument)
      if(strcmp(list->next->key,key)<=0){
        // while the given key is less that the key of the current node, continue traversing
        prev=list;
        list=list->next;
        continue;
      }
      break;
    }
    if(levelNode->level==1){  // if we are at level 1 (bottom level)
      if(strcmp(list->key,key)==0){ // check the node that we stopped the traversing
        // if the key is the same, then we found the key we want. So, return 1 and the date
        (*date)=list->date;
        return 1;
      }else{  // the key does not exist (as every list is sorted)
        (*date)=NULL;
        return 0; //key does not exist
      }
    }else{  // if we are not at the bottom level
      // then we should continue searching from the corresponding node of the down level
      // that node of the below level is passed as argument in the recursive call of search (as explained above in insert function)
      if( prev==NULL && (strcmp(list->key,key)>0) ){
        return search(levelNode->downLevel,levelNode->downLevel->list,key,date);
      }else{
        return search(levelNode->downLevel,list->down,key,date);
      }
    }
  }
}

int searchSL(skipList head,char *key){  // this function is used if we only want to know if a key exists in the given skipList
  // returns 1 if the key exists and 0 otherwise
  // the search is done using the recursive function (search), starting from the first node of the top level
  char *date=NULL;
  int found=search(head->topLevel,head->topLevel->list,key,&date);
  return found;
}

char  *searchGetDateSL(skipList head,char *key){
  // this function is used if we want to know if a key exists and if it exists we want the date vaccinated (only used in vaccinated_persons skipLists)
  // returns NULL if the key does not exist and pointer to the date (that is stored in the bottom level node of the skipList) if the key exists
  char *date=NULL;
  search(head->topLevel,head->topLevel->list,key,&date);
  return date;
}

int delete(slLevelNode *levelNode,slNode *list,slNode *previous,char *key){
  // recursive function in order to delete the given key from the skip list
  // this function is only called by deleteSL function (see below)
  if(list==NULL){ // if skipList is empty
    return 0;
  }else{
    slNode *prev=NULL;
    while(list->next!=NULL){  // start traversing the list of the current level
      // we start traversing from the node that was given by the above level as argument (list)
      if(strcmp(list->next->key,key)<=0){ // compare the keys in order to stop traversing the list
        prev=list;
        list=list->next;
        continue;
      }
      break;
    }
    if(levelNode->level==1){  // if we are at level 1 (bottom level)
      if(strcmp(list->key,key)==0){ // if we found the key
        if(prev==NULL){
          if(strcmp(levelNode->list->key,key)==0){  // if the key is the first node of the list
            levelNode->list=list->next; // attach the second node to the level node (so as to make it the first)
          }else{  // if the key is the first node of the list
            // we must find the previous node of the list, as we started searching from this node
            // as it was given as argument from the above level
            // so as not to start seaching from the start of the list, the above level passes an extra argument (previous)
            // that corresponds to the down node of the previous node of the above level
            // so we dont need to search the whole bottom list, only a few nodes
            slNode *l;
            if(previous==NULL)  // case that the node of the above level was the first node of the level
              l=levelNode->list;
            else
              l=previous;
            while(l->next!=NULL){ // find the previous node
              if(strcmp(l->next->key,key)<0)
                l=l->next;
              else
                break;
            }
            l->next=list->next; // attach the previous node of the list to the next node of the key node that we will delete
          }
        }else{
          prev->next=list->next;
        }
        if(list->date!=NULL){ // if the skip list is a vaccinated_persons skipList
          free(list->date); // also free the date
        }
        free(list);
        return 1;
      }else{
        return 0; //key does not exist
      }
    }else{  // if we are not on the bottom level
      // we must continue searching to the below node with the same key with the node that we stopped the traversing on this level
      // this is accomplished with a recursive call of the insert function (for the below level every time)
      // and by giving as argument the node from which it should start searching the below level
      int deleted;
      if( prev==NULL && (strcmp(list->key,key)>0) ){  //same as explained in insert function
        deleted=delete(levelNode->downLevel,levelNode->downLevel->list,NULL,key);
      }else{
        slNode *downPrev=NULL;
        if(prev!=NULL)
          downPrev=prev->down;  // also give as argument down node of the previous node that we stopped traversing
          // in order to be able to reattach the list when a node is deleted
        deleted=delete(levelNode->downLevel,list->down,downPrev,key);
      }
      // returned from the recursion
      if(deleted){  // if the node with the given key was deleted in the lower level
        if(strcmp(list->key,key)==0){ // if the key also exists in this level
          // delete the node and re-attach the list (explained in lines 269-273)
          if(prev==NULL){
            if(strcmp(levelNode->list->key,key)==0){
              levelNode->list=list->next;
            }else{
              slNode *l;
              if(previous==NULL)
                l=levelNode->list;
              else
                l=previous;
              while(l->next!=NULL){
                if(strcmp(l->next->key,key)<0)
                  l=l->next;
                else
                  break;
              }
              l->next=list->next;
            }
          }else{
            prev->next=list->next;
          }
          free(list);
          return 1;
        }else{
          // if the key does not exist in this level
          return 1;
        }
      }else{
        return 0;
      }
    }
  }
}


void deleteSL(skipList head,char *key){
  //this function is called from database.c file in order to delet a "record" with the given key from the given skipList
  int deleted=delete(head->topLevel,head->topLevel->list,NULL,key); // delete the key
  if(deleted){ // if the key was deleted succesfully
    slLevelNode *levelList=head->topLevel;
    // check if there are empty levels (at the top of the skip list) after the deletion
    // if there are, delete them
    while(levelList!=NULL){
      if(levelList->list==NULL){
        if(levelList->level==1){
          break;
        }
        head->topLevel=levelList->downLevel;
        free(levelList);
        levelList=head->topLevel;
      }else{
        break;
      }
    }
    head->totalLevels=head->topLevel->level;
    head->records--;
  }
  return;
}

void destroySL(skipList head){  // delete/free the given skipList
  slLevelNode *levelsList=head->topLevel;
  while(levelsList!=NULL){  // for evey level of the skipList
    slNode *list=levelsList->list;
    while(list!=NULL){  // for evey node/key in this level
      slNode *next=list->next;
      if(list->date!=NULL){ // only bottom level of vaccinated_persons skipList stores the date, otherwise is NULL
        free(list->date);
      }
      // the key is just a pointer to the key that is stored in the hashTable, so we dont have to free it.
      free(list);
      list=next;
    }
    slLevelNode *nextLevel=levelsList->downLevel;
    free(levelsList);
    levelsList=nextLevel;
  }
  free(head);
}

void printSL(skipList head){  // print the whole skipList (only for debugging reasons, not actually used)
  printf("TOTAL LEVELS %d\n",head->totalLevels);
  printf("_________________________\n");
  for(int i=1;i<=head->totalLevels;i++){
    slLevelNode *level;
    if(i==1){
      level=head->topLevel;
    }else{
      level=level->downLevel;
    }
    printf("*** LEVEL %d: ",level->level);
    slNode *l=level->list;
    while(l!=NULL){
      printf(" %s",l->key);
      l=l->next;
    }
    printf("\n");
  }
}

void listPersons(skipList head){  // print every record that exists in the skipList
  // only called from listNonVaccinatedPersons in database.c file
  slLevelNode *levelsList=head->topLevel;
  while(levelsList!=NULL){  // go to the bottom level
    if(levelsList->level==1){
      break;
    }else{
      levelsList=levelsList->downLevel;
    }
  }
  if(levelsList!=NULL){
    slNode *l=levelsList->list;
    if(l==NULL){
      printf("THERE ARE NO PERSONS RECORDED AS NOT VACCINATED\n");
    }
    while(l!=NULL){ // for every node/key in the bottom level of the skipList
      citizenRecord rec=(citizenRecord)l->down;
      // take the record using the pointer that is stored at the bottom nodes of the skipList
      // and print the record
      printf("%s %s %s %s %d\n",getId(rec),getFirstName(rec),getLastName(rec),getCountry(rec),getAge(rec));
      l=l->next;
    }
  }
}
