#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "database.h"
#include "errorChecking.h"

int bloomSize;  // contains the size of every bloom filter that the user gave

int	main(int argc, char *argv[]){
  if(argc<5){
		printf("Wrong input\n");
		return -1;
	}
  char *fileName; //stores the name of the inputFile
  int count=1;
  int inputRead=0;    // for wrong input check
  while(count<=3){  //read  the arguments of the command line
    // arguments can be read in any sequence
    if(strcmp("-c",argv[count])==0){  // -c argument followed by the inputFile name
      fileName=malloc(sizeof(char)*(strlen(argv[count+1])+1));
      strcpy(fileName,argv[count+1]);
      inputRead++;
      count+=2;
      continue;
    }
    if(strcmp("-b",argv[count])==0){  // -b for bloomFilter size
      bloomSize=atoi(argv[count+1]);
      inputRead++;
      count+=2;
      continue;
    }
    count+=2;
  }
  if(inputRead!=2){    // check if both arguments were read
    printf("Wrong input\n");
    return 0;
  }

  Data database=initializeData(); // initialize all the data
  readCitizenRecordsFile(database,fileName);  // read all the records from the inputFile and insert them on the database
  free(fileName); // free the file name as we dont need it any more
  printf("All the records from the input file were inserted\n");

  printf("________________________ GIVE YOUR COMMANDS ________________________\n\n");
  // promt is starting in order to recieve commands from the user
  printf(">: ");
  char buffer[1024];
  char function[40];
  if(fgets(buffer,1024,stdin)==NULL){   //get the first command that the user gave
    printf("Error\n");
    return -1;
  }
  sscanf (buffer,"%s \n",function);   //get only the first word that the user wrote(the function that user wants to call)

  while(strcmp(function,"/exit")!=0){  // read commands from user, until he gives exit command
    // below there are all the cases that the user can call a function
    // **
    // NOTE: for every function that is called in order to execute the wanted command, check database.c for the implementation and explanation
    // **
    if(strcmp(function,"/vaccineStatusBloom")==0){  // /vaccineStatusBloom command
      char citizenID[20],virusName[50],wrongInput[50];
      // split the line into arguments. vaccineStatusBloom always need two arguments (citizenID and virusName)
      if(sscanf(buffer,"%s %s %s %s\n",function,citizenID,virusName,wrongInput)==3){
        // if the input has the right number of arguments
        if(validVirus(virusName))
          vaccineStatusBloom(database,citizenID,virusName);
        else
          printf("Wrong input - Invalid virus name.\n");
      }else{    // if user doesn't give 3 data as written in scanf print error message and do nothing
        printf("Wrong input.\n");
      }
    }else if(strcmp(function,"/vaccineStatus")==0){ // /vaccineStatus command
      char citizenID[50],virusName[50],wrongInput[50];
      // split the line into arguments. vaccineStatusBloom can have either 2 or 3 arguments (if virus is not given)
      int read=sscanf(buffer,"%s %s %s %s\n",function,citizenID,virusName,wrongInput);
      if(read==3){  // if virus is given
        if(validVirus(virusName))
          vaccineStatus(database,citizenID,virusName);
        else
          printf("Wrong input - Invalid virus name.\n");
      }else if(read==2){  // if virus is NOT given
        vaccineStatusAll(database,citizenID);
      }else{  // in every other case the input is wrong
        printf("Wrong input.\n");
      }
    }else if(strcmp(function,"/populationStatus")==0){  // /populationStatus command
      char arg1[50],arg2[50],arg3[50],arg4[50],wrongInput[50];
      // in populationStatus, user can give multiple possible number of arguments (see every case below)
      // in the cases that one (or more) arguments are not given, NULL is passed to the function at the corresponding argument
      int read=sscanf(buffer,"%s %s %s %s %s %s\n",function,arg1,arg2,arg3,arg4,wrongInput);
      if(read==5){  // if all arguments are given ( country virus date1 date2 )
        if(validCountryOrName(arg1) && validVirus(arg2) && validDate(arg3) && validDate(arg4))  // check if the last 2 arguments are valid dates
          populationStatus(database,arg1,arg2,arg3,arg4);
        else
          printf("ERROR - INVALID DATES\n");
      }else if(read==4){  // if country was NOT given and only ( virus date1 date2 ) was given
        if(validVirus(arg1) && validDate(arg2) && validDate(arg3))  // check if the last 2 arguments are actually dates
          populationStatus(database,NULL,arg1,arg2,arg3);
        else  // if at least one of the last 2 arguments is not a valid date, print error message
          printf("ERROR - INVALID DATES\n");
      }else if(read==3){  // if dates were NOT given and only ( country virus ) was given
          if(validCountryOrName(arg1) && validVirus(arg2))  // check if the first argument is actually a country
            populationStatus(database,arg1,arg2,NULL,NULL);
          else  // if the first argument is not a valid country, , print error message
            printf("ERROR - WRONG INPUT\n");
      }else if(read==2){  // if country and dates were NOT given and only ( virus ) was given
        if(validVirus(arg1))
          populationStatus(database,NULL,arg1,NULL,NULL);
        else
          printf("ERROR - WRONG INPUT\n");
      }else{  // in every other case, the input is wrong
        printf("Wrong input.\n");
      }
    }else if(strcmp(function,"/popStatusByAge")==0){  // /popStatusByAge command
      // in popStatusByAge, user can give multiple possible number of arguments (same cases as /populationStatus)
      // in the cases that one (or more) arguments are not given, NULL is passed to the function at the corresponding argument
      char arg1[50],arg2[50],arg3[50],arg4[50],wrongInput[50];
      int read=sscanf(buffer,"%s %s %s %s %s %s\n",function,arg1,arg2,arg3,arg4,wrongInput);
      if(read==5){  // if all arguments are given ( country virus date1 date2 )
        if(validCountryOrName(arg1) && validVirus(arg2) && validDate(arg3) && validDate(arg4))  // check if the last 2 arguments are valid dates
          popStatusByAge(database,arg1,arg2,arg3,arg4);
        else
          printf("ERROR - INVALID DATES\n");
      }else if(read==4){  // if country was NOT given and only ( virus date1 date2 ) was given
        if(validVirus(arg1) && validDate(arg2) && validDate(arg3))  // check if the last 2 arguments are actually dates
          popStatusByAge(database,NULL,arg1,arg2,arg3);
        else
          printf("ERROR - INVALID DATES\n");
      }else if(read==3){  // if dates were NOT given and only ( country virus ) was given
          if(validCountryOrName(arg1) && validVirus(arg2))   // check if the first argument is actually a country
            popStatusByAge(database,arg1,arg2,NULL,NULL);
          else
            printf("ERROR - WRONG INPUT\n");
      }else if(read==2){  // if country and dates were NOT given and only ( virus ) was given
        if(validVirus(arg1))
          popStatusByAge(database,NULL,arg1,NULL,NULL);
        else
          printf("ERROR - WRONG INPUT\n");
      }else{  // in every other case, the input is wrong
        printf("Wrong input.\n");
      }
    }else if(strcmp(function,"/insertCitizenRecord")==0){   // /insertCitizenRecord command
      char citizenID[20],firstName[50],lastName[50],country[50],virusName[50],vaccinated[10],date[15],wrongInput[50];
      int age;
      // split the line into arguments. insertCitizenRecord needs either 8 arguments (when YES is given) either 7 arguments (when NO is given)
      int read=sscanf(buffer,"%s %s %s %s %s %d %s %s %s %s\n",function,citizenID,firstName,lastName,country,&age,virusName,vaccinated,date,wrongInput);
      if(read==9){  // if the total arguments are 9 (including /insertCitizenRecord)
        // only YES can be given at vacinated argument
        if(strcmp(vaccinated,"YES")==0){  // correct input
          if(validCountryOrName(firstName) && validCountryOrName(lastName) && validCountryOrName(country) && validAge(age) && validVirus(virusName) && validDate(date))
            insertCitizenRecord(database,citizenID,firstName,lastName,country,age,virusName,vaccinated,date);
          else
            printf("Wrong input - Error in record data\n");
        }else{
          printf("Wrong input.\n");
        }
      }else if(read==8){  // if the total arguments are 8 (including /insertCitizenRecord)
        // the only right input is when NO is given at vaccinated argument
        if(strcmp(vaccinated,"NO")==0){ // correct input
          if(validCountryOrName(firstName) && validCountryOrName(lastName) && validCountryOrName(country) && validAge(age) && validVirus(virusName))
            insertCitizenRecord(database,citizenID,firstName,lastName,country,age,virusName,vaccinated,NULL);
          else
            printf("Wrong input.\n");
        }else{
          printf("Wrong input.\n");
        }
      }else{  // in every other case, the input is wrong
        printf("Wrong input.\n");
      }
    }else if(strcmp(function,"/vaccinateNow")==0){  // /vaccinateNow command
      char citizenID[20],firstName[50],lastName[50],country[50],virusName[50],wrongInput[50];
      int age;
      // split the line into arguments. vaccinateNow always needs 7 arguments (including /vaccinateNow)
      int read=sscanf(buffer,"%s %s %s %s %s %d %s %s\n",function,citizenID,firstName,lastName,country,&age,virusName,wrongInput);
      if(read==7){  // if the input actually contains 7 arguments (6 + the command)
        if(validCountryOrName(firstName) && validCountryOrName(lastName) && validCountryOrName(country) && validAge(age) && validVirus(virusName))
          vaccinateNow(database,citizenID,firstName,lastName,country,age,virusName);
        else
          printf("Wrong input.\n");
      }else{  // in every other case the input is wrong
        printf("Wrong input.\n");
      }
    }else if(strcmp(function,"/list-nonVaccinated-Persons")==0){  // /list-nonVaccinated-Persons command
      char virusName[50],wrongInput[50];
      // split the line into arguments. /list-nonVaccinated-Persons always needs 2 arguments (including /list-nonVaccinated-Persons)
      // as only the name of the virus is given
      int read=sscanf(buffer,"%s %s %s\n",function,virusName,wrongInput);
      if(read==2){
        if(validVirus(virusName))
          listNonVaccinatedPersons(database,virusName);
        else
          printf("Wrong input.\n");
      }else{
        printf("Wrong input.\n");
      }
    }else{  // in every other case the user input was wrong
      printf("Wrong input.\n");
    }

    printf(">: ");
    // read the next command from the user
    if(fgets(buffer,1024,stdin)==NULL){
      printf("Error\n");
      return -1;
    }
    sscanf (buffer,"%s \n",function); // get only the first word that the user gave
  }


  deleteData(database);   // delete/free all the data before exiting
  return 0;
}
