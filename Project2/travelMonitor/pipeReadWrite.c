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

extern int buffSize;
extern int bloomSize;

// all the messages sent between the travelMonitor and the monitors are sent by using these function
// the only messages that are sent are strings, bloomFilters and integers (when sending the length of a string)
// every time we send at most buffSize bytes. So if a message is bigger than buffSize we send it in "packages" with multiple write calls
// on the other side symmetric writes are done in order to read the message

void sendInt(int fd,int message){ // sends an integer
  int length=sizeof(int); // get the size of an int
  int currLen = 0;  // stores how many bytes have been sent
  while(currLen<length){  // start sending the int
    if((length-currLen)<buffSize){  // if the remaining bytes/length are smaller than the buffSize send only the remaining bytes
      if( (currLen += write(fd,(&message),(length-currLen))) == -1 ){ perror("write: "); exit(-1); } // send the int starting from the byte the previous write ended
    }else{  // if the remaining bytes/length are bigger or equal than the buffSize send a message of size buffSize
      if( (currLen += write(fd,(&message),buffSize)) == -1 ){ perror("write: "); exit(-1); } // send the int starting from the byte the previous write ended
      message = message >> (8*buffSize);  // shift the numbers, as many byte as the bytes that we sent at the last write call
      // therefore the next write will start "writing" the next byte
    }
  }
}

int readInt(int fd){
  int length=sizeof(int);
  int currLen = 0;  // stores how many bytes have been read
  int message=0;
  int tempMessage=0;
  while(currLen<length){   // while we dont have read the whole int
    int oldCurrLen=currLen;
    if((length-currLen)<buffSize){  // if the remaining bytes that we expect to read are smaller than the buffSize read only the remaining bytes
      if( (currLen += read(fd,(&tempMessage),(length-currLen))) == -1 ){ perror("read: "); exit(-1); } // only read the remaining bytes of the int
    }else{  // if the remaining bytes/length are bigger or equal than the buffSize read buffSize bytes
      if( (currLen += read(fd,(&tempMessage),buffSize)) == -1 ){ perror("read: "); exit(-1); } //read buffSize bytes
    }
    message += tempMessage << (8*oldCurrLen); // shift the number that we read, in order to correspond to the byte of the int that we read
    tempMessage=0;
  }
  return message; // after reading the whole int (4 bytes) return the number;
}

void sendMessage(int fd,char *message){ // sends a string message
  int length=strlen(message); // find the length of the string
  sendInt(fd,length);  // send only the length of the string
  int currLen = 0;  // stores how many bytes have been sent
  while(currLen<length){  // start sending the string
    if((length-currLen)<buffSize){  // if the remaining bytes/length are smaller than the buffSize send only the remaining bytes
      if( (currLen += write(fd,message+currLen,(length-currLen))) == -1 ){ perror("write: "); exit(-1); } // send the string starting from where the previous write ended
    }else{  // if the remaining bytes/length are bigger or equal than the buffSize send a message of size buffSize
      if( (currLen += write(fd,message+currLen,buffSize)) == -1 ){ perror("write: "); exit(-1); } // send the string starting from where the previous write ended
    }
  }
}

char *readMessage(int fd){  // reads a string message
  int length= readInt(fd); // read the length of the string that will be read
  char message[length+1]; //the final string will be stored here
  int currLen = 0;  // how many bytes have been read
  int i=0;
  while(currLen<length){  // while we dont have read the whole string
    char tempbuff[buffSize+1];
    if((length-currLen)<buffSize){  // if the remaining bytes of the string are less from the buffSize
      int oldCurrLen = currLen;
      if( (currLen += read(fd,tempbuff,(length-currLen))) == -1 ){ perror("read: "); exit(-1); }  // only read the remaining bytes
      tempbuff[(length-oldCurrLen)]='\0'; // add '\0' at the end of the string that was just read
    }else{
      if( (currLen += read(fd,tempbuff,buffSize)) == -1 ){ perror("read: "); exit(-1); }  // read a string of buffSize length
      tempbuff[buffSize]='\0';  // add '\0' at the end of the string that was just read
    }
    if(i==0)
      strncpy(message,tempbuff,strlen(tempbuff)+1); // after the first read copy the read string to the message buffer
    else
      strncat(message,tempbuff,strlen(tempbuff)+1); // concat the new string to the previous string
    i++;
  }
  // we have read the whole string
  char *receivedMessage=malloc(sizeof(char)*(strlen(message)+1)); // allocate space in order to store the string
  strcpy(receivedMessage,message);  // copy the string
  return receivedMessage; // return the string
}

// for the bloomFilters the same logic as the strings is followed. A bloomFilter is an array of bytes
// we dont need to first send the size of the bloomFilter (like we did for strings) because it is always the same and it is known to both the travelMonitor and the monitors

void sendBloom(int fd,bloomFilter bf){  // sends a bloomFilter
  byte *array=getBloomArray(bf);  // get the bloomFilter array
  int bytesSent = 0;  // how many bytes have been sent
  while(bytesSent<bloomSize){ // while there are bytes that have not sent yet
    if((bloomSize-bytesSent)<buffSize){ // if the reamaining bytes are less than the buffSize
      if( (bytesSent += write(fd,array+bytesSent,(bloomSize-bytesSent))) == -1 ){ perror("write: "); exit(-1); }  // only send the remaining bytes  starting from where the previous write ended
    }else{  // if the reamaining bytes are more than the buffSize
      if( (bytesSent += write(fd,array+bytesSent,buffSize)) == -1 ){ perror("write: "); exit(-1); } // send buffSize bytes starting from where the previous write ended
    }
  }
}

bloomFilter readBloom(int fd){  // reads a bloomFilter
  byte array[bloomSize];  // the bloomFilter will be stored here
  int bytesRead = 0;  // how many bytes have been read
  while(bytesRead<bloomSize){ // while we dont have read the whole array
    byte tempArray[buffSize];
    int oldBytesRead = bytesRead;
    int currentRead;  // how many bytes where read at a read call
    if((bloomSize-bytesRead)<buffSize){ // if the remaining bytes of the array are less from the buffSize
      if( (bytesRead += read(fd,tempArray,(bloomSize-bytesRead))) == -1 ){ perror("read: "); exit(-1); }  // only read the remaining bytes
      currentRead = bloomSize-oldBytesRead; // how many bytes where read with the above read call
    }else{
      if( (bytesRead += read(fd,tempArray,buffSize)) == -1 ){ perror("read: "); exit(-1); } // read buffSize bytes
      currentRead = buffSize; // how many bytes where read with the above read call
    }
    int j=0;
    for(int i=oldBytesRead;i<oldBytesRead+currentRead;i++){ // copy the bytes we just read to the bloomFilter
      array[i]=tempArray[j++];
    }
  }
  // create a bloomFilter struct with the bloomFilter array that we just read
  bloomFilter bf=setBloom(array,bloomSize);
  return bf;  // return the bloomFilter struct
}
