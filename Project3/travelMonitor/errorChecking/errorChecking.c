#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int inRange(int num, int lb, int ub){
  return (num>=lb && num<=ub);
}

int validDate(const char dateOrig[]){ // this function checks if the given string is a valid date
  // a valid date must contain at most 10 characters, and only contain numbers and '-'
  // returns 1 if the date is valid and 0 otherwise
  char date[20];
  strcpy(date,dateOrig);
  int len=strlen(date);
  int count=0;
  if(len>10 || len<8){
    return 0;
  }
  for (int i=0;i<len;i++){
    if((date[i]>='0' && date[i]<='9')){
      continue;
    }else if(date[i]=='-'){
      count++;
      continue;
    }else{
      return 0;
    }
  }
  if(count==2){
    if(len==10 && date[2]=='-' && date[5]=='-'){
      char *day = strtok(date,"-"); //split the date string and get the day
      char *month = strtok(NULL,"-"); //get the month
      char *year = strtok(NULL,"-");  //get the year
      // the check if day is in range [1,30] and month is in rage [1,12] and year is in range [1700,2021]
      return (inRange(atoi(day),1,30) && inRange(atoi(month),1,12) && inRange(atoi(year),1700,2021));
    }else if( len==9 && ( (date[1]=='-' && date[4]=='-') || (date[2]=='-' && date[4]=='-') ) ){
      char *day = strtok(date,"-");
      char *month = strtok(NULL,"-");
      char *year = strtok(NULL,"-");
      return (inRange(atoi(day),1,30) && inRange(atoi(month),1,12) && inRange(atoi(year),1700,2021));
    }else if(len==8 && date[1]=='-' && date[3]=='-'){
      char *day = strtok(date,"-");
      char *month = strtok(NULL,"-");
      char *year = strtok(NULL,"-");
      return (inRange(atoi(day),1,30) && inRange(atoi(month),1,12) && inRange(atoi(year),1700,2021));
    }else{
      return 0;
    }
  }else{
    return 0;
  }
}

int validCountryOrName(const char string[]){  // this function checks if the given string is a valid country
  // a valid country must only constist from alphabet characters and not contain spaces
  // returns 1 if the country is valid and 0 otherwise
  int len=strlen(string);
  for (int i=0;i<len;i++){
    if((string[i]>='a' && string[i]<='z') || (string[i]>='A' && string[i]<='Z')){
      continue;
    }else{
      return 0;
    }
  }
  return 1;
}

int validVirus(const char virus[]){  // this function checks if the given string is a valid virus
  // a valid virus must only constist from alphabet characters, numbers and at most 1 '-'
  // returns 1 if the virus is valid and 0 otherwise
  int len=strlen(virus);
  int count=0;
  for (int i=0;i<len;i++){
    if((virus[i]>='a' && virus[i]<='z') || (virus[i]>='A' && virus[i]<='Z') || (virus[i]>='0' && virus[i]<='9')){
      continue;
    }else if(virus[i]=='-'){
      if(count==0){ // check how many '-' exist in the virus name
        count++;
        continue;
      }else{  // if virus contains more than one '-', then it's not valid
        return 0;
      }
    }else{
      return 0;
    }
  }
  return 1;
}

int validAge(int age){
  return ( (age>0) && (age<=120) );
}
