#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef uint8_t byte;

#define K 16
// for the implementation of the bloom filter i am using the type uint8_t which size is 1byte (every uint8_t has 8 bits)
// the bloom filter is an array of uint8_t elements
// the size of the bloom filter is given by the user in bytes
// Therefore the bloom filter is an uint8_t array of the given size
typedef struct bfnode{
  uint8_t *bf; //the array of the bloom filter
  int size; //size of the bloom filter (in bytes)
}bloomNode;
typedef bloomNode *bloomFilter;

byte *getBloomArray(bloomFilter bloom){
  return bloom->bf;
}

bloomFilter setBloom(byte *array,int bloomSize){  // create a bloom filter node, with the given bloom filter array
  bloomFilter bloom=(bloomFilter)malloc(sizeof(bloomNode)); // create the head/dummy node
  bloom->bf=(uint8_t *)malloc(bloomSize*sizeof(uint8_t));  // create the actual bloom filter (array of bytes)
  for(int i=0;i<bloomSize;i++){ // copy the bloomFilter
    bloom->bf[i]=array[i];
  }
  bloom->size=bloomSize;   // store the size
  return bloom; // return the bloomFilter node
}

// HASH FUNCTIONS /////
unsigned long djb2(unsigned char *str) {
	unsigned long hash = 5381;
	int c;
	while ((c = *str++)) {
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	}
	return hash;
}
unsigned long sdbm(unsigned char *str) {
	unsigned long hash = 0;
	int c;

	while ((c = *str++)) {
		hash = c + (hash << 6) + (hash << 16) - hash;
	}

	return hash;
}
unsigned long hash_i(unsigned char *str, unsigned int i) {
	return djb2(str) + i*sdbm(str) + i*i;
}
/////////////////////////

void changeBit(bloomFilter bloomHead,int hashKey){  // set the given bit to 1
  uint8_t *bloom=bloomHead->bf; // take the array
  // NOTE: bit 0 corresponds to the left bit of the byte as shown in the example of the excercise
  uint8_t mask= 1 << (7-hashKey%8); //create a byte (8bits) that only has 1 in the bit we want to set
  // hashKey/8 finds in which position of the array the bit we want set is
  // hashKey%8 corresponds to the bit we want to set to 1 on the byte that hashKey/8 corresponds to
  bloom[hashKey/8]=bloom[hashKey/8] | mask;   // set that bit to 1
}

void bloomInsert(bloomFilter bloom,char *id){ // insert a new record to the bloom filter
  for(int i=0;i<K;i++){  // for every hash function
    unsigned long hash=hash_i((unsigned char *)id,i); // find the bit we want to set, using the hash function
    // hash%(bloom->size*8) corresponds to the bit of the bloom filter that we want to set to 1
    changeBit(bloom,hash%(bloom->size*8));  // set the bit to 1
  }
}

int bloomExists(bloomFilter bloomHead,char *id){ // check if an id has been inserted to the bloom filter
  uint8_t *bloom=bloomHead->bf;
  int flag=1;
  for(int i=0;i<K;i++){  // for every hash function
    unsigned long hash=hash_i((unsigned char *)id,i); //get the hash value for the given id
    hash=hash%(bloomHead->size*8);  // find which bit of the bloom filter to check
    uint8_t byte=bloom[((int)hash)/8];  // take the byte that the bit we want belong to
    int isSet=((byte) & (1<<(7-(hash%8)))); // check if that bit is set to 1
    // NOTE: bit 0 corresponds to the left bit of the byte as shown in the example of the excercise
    if(!isSet){ // if the bit is not set
      // then the record surely has not been inserted to the bloom filter
      flag=0; // so set the flag to 0
      break;  // and stop
    }
  }
  return flag;
}

bloomFilter createBloom(int size){  // create/initialize the bloom filter
  bloomFilter bloom=(bloomFilter)malloc(sizeof(bloomNode)); // create the head/dummy node
  bloom->bf=(uint8_t *)malloc(size*sizeof(uint8_t));  // create the actual bloom filter (array of bytes)
  for(int i=0;i<size;i++){    // initialize all bits to 0
    bloom->bf[i]=0;
  }
  bloom->size=size;   // store the size
  return bloom;
}

void deleteBloom(bloomFilter bloom){  // delete/free the bloom filter
  free(bloom->bf);
  free(bloom);
}
