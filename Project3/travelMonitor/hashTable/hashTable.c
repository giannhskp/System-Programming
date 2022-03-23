#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct node{
  char *citizenId;
  char *firstName;
  char *lastName;
  char *country;
  int age;
  struct node *next;
}Node;  //list node
typedef Node *citizenRecord;    //pointer to the list node
typedef citizenRecord *Hash;  //double pointer to the list node (array of lists)

typedef struct hash{
  int size;   // the size of the hashTable
  int count;    //how many citizens are in the HashTable
  Hash table;   //the HashTable
}HashHeadNode;    // dummy node for hashTable
typedef HashHeadNode *HashTable;    //pointer to the dummy node

char *getId(citizenRecord node){
  return node->citizenId;
}
char *getFirstName(citizenRecord node){
  return node->firstName;
}
char *getLastName(citizenRecord node){
  return node->lastName;
}
char *getCountry(citizenRecord node){
  return node->country;
}
int getAge(citizenRecord node){
  return node->age;
}

HashTable htCreate(int size){ //  create/initialize the hash table
  HashTable head=malloc(sizeof(HashHeadNode));  //Create hash table dummy node
  head->size=size;    //store the given size of the hash table
  head->table=malloc(head->size*(sizeof(citizenRecord)));   //create a hash table of the given size
  for(int i=0;i<head->size;i++){
    head->table[i]=NULL;    //initialize hash table with null in every space (lists)
  }
  head->count=0;
  return head;
}

citizenRecord newNode(char *citizenId,char *firstName,char *lastName,char *country,int age){   // creates a node of a citizen
  citizenRecord node=malloc(sizeof(Node));
  node->citizenId=malloc(sizeof(char)*(strlen(citizenId)+1));
  strcpy(node->citizenId,citizenId);
  node->firstName=malloc(sizeof(char)*(strlen(firstName)+1));
  strcpy(node->firstName,firstName);
  node->lastName=malloc(sizeof(char)*(strlen(lastName)+1));
  strcpy(node->lastName,lastName);
  node->country=country;  // a pointer to the country is kept, in order to avoid duplicate info (country is only stored one time in the country list)
  node->age=age;
  node->next=NULL;
  return node;
}

void deleteNode(citizenRecord node){  // delete a citizen node
  free(node->citizenId);
  free(node->firstName);
  free(node->lastName);
  free(node);
}

unsigned int hash(HashTable ht,char *id){      //find the hash table index for the given id
  unsigned long int hashcode = 0;
  int a=33;
  char *tempkey=id;
  for (; *tempkey!='\0'; tempkey++)   // for every char/number of the id
    hashcode=(a*hashcode + *tempkey) % ht->size;
  return hashcode;    //hash code is beiing %hashTableSize so it is 0<= hashcode <= ht->size
}

void htResizeInsert(HashTable ,citizenRecord );

void htResize(HashTable ht){  // the hash table is resizable, when it is 90% full this function is called
  // in order to double the size of the hash table, so as to retain the search/insert in O(1) time
  Hash oldTable=ht->table;  // store the old hashTable
  int oldSize=ht->size; // store the size of the old hashTable
  int newSize=2*(ht->size); // the new size is double the preivous one
  Hash newTable=malloc(newSize*(sizeof(citizenRecord)));  //create the new hashTable
  for(int i=0;i<newSize;i++){ // initialize it
    newTable[i]=NULL;
  }
  ht->table=newTable; // refresh the dummy node with the new hash table
  ht->size=newSize;   // and the new size
  ht->count=0;
  // insert all the records of the old hash table to the new hash table
  for (int i = 0; i < oldSize; ++i){   //for every bucket of the old hash table
    citizenRecord slot=oldTable[i];    //the "overflow list" of this bucket
    if(slot==NULL){
        continue;
    }
    while(slot!=NULL){   //for every node (every citizen) of the overflow list
        citizenRecord next=slot->next;
        htResizeInsert(ht,slot);  // insert it to the new hash table
        // a new insert function is created for the resize so as not to delete every node of the old hash table
        // and create a new node for the new hash table (see below: htResizeInsert)
        slot=next;
    }
  }
  free(oldTable);
}

void htResizeInsert(HashTable ht,citizenRecord rec){
  // this function is created in order to avoid extra memmory allocation and frees
  // with this functions the same nodes from the old hash table are used for the new hash table
  // in this way no extra allocation are done during the resize of the hash table
  int index=hash(ht,rec->citizenId);
  citizenRecord slot=ht->table[index];
  if(slot==NULL){   //no collitions
    ht->table[index]=rec; //insert it to the hash table, as the first (and only) node of the list in this "Bucket"
    rec->next=NULL;
    (ht->count)++;
    return;
  }else{    //if there are already citizens in this "bucket"
    citizenRecord prev;
    while(slot!=NULL){
      if(strcmp(getId(slot),rec->citizenId)==0){    //check if the citizen already exists
        return;
      }
      prev=slot;
      slot=slot->next;
    }
    //when we reach the last node of the list (the citizen does not exist)
    prev->next=rec;    //connect it to the last node of the list
    rec->next=NULL;
    (ht->count)++;
    return;
  }
}

citizenRecord htSearchOrInsert(HashTable ht,char *citizenId,char *firstName,char *lastName,char *country,int age){
  if((double)(ht->count)>(0.9*((double)((ht->size))))){ // if the hash table is 90% full, resize it in order to keep the search/insert O(1)
      htResize(ht);
  }
  int index=hash(ht,citizenId); // find in which bucket the new record should go in
  citizenRecord slot=ht->table[index];  // get the overflow list of this bucket
  if(slot==NULL){   //no collitions so just insert the node with the citizen
    citizenRecord newnode=newNode(citizenId,firstName,lastName,country,age);     //create the node of the hashTable list that contains the given citizen
    ht->table[index]=newnode; //insert it to the hash table, as the first (and only) node of the list in this "Bucket"
    (ht->count)++;
    return newnode;
  }else{    //if there are already citizens in this "bucket"
    citizenRecord prev;
    while(slot!=NULL){
      if(strcmp(getId(slot),citizenId)==0){    //check if the citizen already exists
        if( (strcmp(getFirstName(slot),firstName)==0) && (strcmp(getLastName(slot),lastName)==0) && (strcmp(getCountry(slot),country)==0) && (getAge(slot)==age) ){
          return slot;
        }else{
          return NULL;
        }
      }
      prev=slot;
      slot=slot->next;
    }
    //when we reach the last node of the list (the citizen does not exist)
    citizenRecord newnode=newNode(citizenId,firstName,lastName,country,age); //create the node of the hashTable list that contains the given citizen
    prev->next=newnode;    // and connect it to the last node of the list
    (ht->count)++;
    return newnode;
  }
}

citizenRecord htSearch(HashTable ht,char *citizenId){
  int index=hash(ht,citizenId);   //find in wich bucket the citizen should be (using hash function)
  citizenRecord slot=ht->table[index];    //go to this "bucket"
  if(slot==NULL){   //no citizen exists in this slot, so it is sure that citizenId does not exist in the hash table
    return NULL;
  }else{
    while(slot!=NULL){    //check every citizen in this slot (all the collitions)
      if(strcmp(getId(slot),citizenId)==0){   //if the citizen is found, return a pointer to the node
          return slot;   //we found the citizen so return a pointer to the citizen
      }
      slot=slot->next;
    }
    //if we traverse the whole list in this bucket and we dont find the citizen, the citizen does not exist in the hash table
    return NULL;
  }
}

void htDestroy(HashTable ht){   //destroys and frees the hash table (and the citizen nodes)
  if(ht==NULL)
    return;
  for (int i=0;i<ht->size;++i){   //for every bucket in the hash table
    citizenRecord slot=ht->table[i];    //take the citizen list
    if(slot==NULL){
        continue;
    }
    for(;;){   //every node in the citizen list
        if(slot->next==NULL){   //for the last node
            deleteNode(slot);     // deleteNode()  deletes the citizen info and the list node itself
            break;
        }
        citizenRecord next=slot->next;
        deleteNode(slot);
        slot=next;
    }
  }
  free(ht->table);    //free the board
  free(ht);   //free the dummy node of the hash table
}
