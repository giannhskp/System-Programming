#!/bin/bash

if (($#!=4)); then  #check if the correct number of arguments was given
  echo "Wrong input"
  exit
fi
virusesFile=$1
countriesFile=$2
numLines=$3
duplicatesAllowed=$4

if [ ! -e "$virusesFile" ]; then  # check if the virus file actually exists
  echo "$virusesFile does not exist"
  exit
fi
if [ ! -e "$countriesFile" ]; then  # check if the country file actually exists
  echo "$countriesFile does not exist"
  exit
fi
if [[ ${numLines} =~ [a-zA-Z] ]] ; then   # check if the numLines is actually an integer
  echo "Wrong input: numLines not a number"
  exit
fi
if [[ ${duplicatesAllowed} =~ [a-zA-Z] ]] ; then  # check if the duplicatesAllowed is actually an integer
    duplicatesAllowed=1   # if not, set duplicatesAllowed as 1
fi
if (( ${duplicatesAllowed} != 0 )); then
  duplicatesAllowed=1
fi


fileName="inputFile"   #the output file that will be created
if [ -e $fileName ]; then   # if it already exists, delete it
   rm $fileName
fi

lineCount=1

countries=()
while read -r line; do  # read the countries from the given file
  countries+=($line)  # and store them in an array
done < $countriesFile
viruses=()
while read -r line; do  # read the viruses from the given file
  viruses+=($line)  # and store them in an array
done < $virusesFile

idBoard=($(shuf -i 0-9999 -n ${numLines}))  # create an array with numLines different ids in range 0-9999 (at most 4 characters)
alphabet="abcdefghijklmonpqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ" # the charactert that a firstName/lastName is allowed to have
idCount=0 # counter of the unique records created (the current index of the idBoard)
duplicates=()

while [ $lineCount -le $numLines ]; do  # create records until the wanted number of records is reached
    if((${idCount}>=${#idBoard[@]})); then
      if (($duplicatesAllowed==0));then # if duplicates are not allowed, we can create 10K unique records maximum
        echo "Can not generate more unique id's with max length 4. Generated the maximum unique records possible"
        exit
      else
        # if records for all unique id's were created (possibly with some duplicates)
        # start getting randomly citizens data and create new duplicates until we reach the wanted number of records
        while [ $lineCount -le $numLines ]; do
          citizenData=${duplicates[$RANDOM % ${#duplicates[@]}]}
          virus=${viruses[$RANDOM % ${#viruses[@]} ]} # pick a random virus from the virus array
          if (( (($RANDOM % 2)) == 0 )); then # decide if the record will contain YES or NO
            day=$((($RANDOM % 30)+1))
            month=$((($RANDOM % 12)+1))
            year=$((($RANDOM % 22)+2000))
            date="$day-$month-$year"
            record="$citizenData $virus YES $date"  # create the record by appending all the random string generated before
          else
            # if it will contain NO
            record="$citizenData $virus NO"
          fi
          echo $record >> $fileName # write the record to the file
          lineCount=$((lineCount + 1))
        done
        # the wanted records were created
        exit
      fi
    fi
  citizenId=${idBoard[${idCount}]}  #get the next unique id
  idCount=$((idCount + 1))

  nameLength=$((($RANDOM % 10)+3))  # generate a random length for the firstName
  #firstName=$(shuf -zer -n${nameLength}  {A..Z} {a..z})  # this does the same thing as the loop, but in some bash versions it prints warning about the '\0' char, so it is not used
  firstName=""
  for (( i=1; i <= $nameLength; i++ )); do
  #for i in `seq 1 $nameLength`; do  # create a random firstName whith lentgh: nameLength
      char=${alphabet:$RANDOM % 52:1} # pick a random character from the alphabet
      firstName+=$char  # append the random character to the name
  done
  # do the same thing for the lastName
  nameLength=$((($RANDOM % 10)+3))
  #lastName=$(shuf -zer -n${nameLength}  {A..Z} {a..z})
  lastName=""
  for (( i=1; i <= $nameLength; i++ )); do
  # for i in `seq 1 $nameLength`; do
      char=${alphabet:$RANDOM % 52:1}
      lastName+=$char
  done

  country=${countries[$RANDOM % ${#countries[@]} ]} # pick a random country from the country array
  virus=${viruses[$RANDOM % ${#viruses[@]} ]} # pick a random virus from the virus array
  age=$((($RANDOM % 120)+1))  # pick a random age
  if (( (($RANDOM % 2)) == 0 )); then # decide if the record will contain YES or NO
    # if it will contain YES, generate a random date
    day=$((($RANDOM % 30)+1))
    month=$((($RANDOM % 12)+1))
    year=$((($RANDOM % 22)+2000))
    date="$day-$month-$year"
    record="$citizenId $firstName $lastName $country $age $virus YES $date"  # create the record by appending all the random string generated before
  else
    # if it will contain NO
    record="$citizenId $firstName $lastName $country $age $virus NO"
  fi
  echo $record >> $fileName # write the record to the file
  lineCount=$((lineCount + 1))
  if (($duplicatesAllowed==0));then # if we dont want duplicates, continue from the start of the loop so as to generate a new unique record
    continue
  fi
  duplicates+=("$citizenId $firstName $lastName $country $age")
  # if we want duplicates
  possibility=4 # possibility 25% to create a duplicate for every record
  # so as the duplicate records have the same id,firstName,lastName,country,age we keep those data and only change virus,vaccinated and date
  while [ $lineCount -le $numLines ];do # check if we have reached the wanted number of records
    if (( (($RANDOM % $possibility)) == 0 )) || (( $lineCount == 2 )); then  # decide if we will create a duplicate
      # if we will create a duplicate for this record
      newVirus=${viruses[$RANDOM % ${#viruses[@]} ]} # pick a random virus
      if (( (($RANDOM % 2)) == 0 )); then # decide if the record will contain YES or NO
        # if it will contain YES, generate a random date
        newDay=$((($RANDOM % 30)+1))
        newMonth=$((($RANDOM % 12)+1))
        newYear=$((($RANDOM % 22)+2000))
        newDate="$newDay-$newMonth-$newYear"
        dupRecord="$citizenId $firstName $lastName $country $age $newVirus YES $newDate"  # create the record by appending all the random string generated before
        # we use the same citizenId,firstName,lastName,country,age that were generated before this loop
      else
        # if it will contain NO
        dupRecord="$citizenId $firstName $lastName $country $age $newVirus NO"
      fi
      echo $dupRecord >> $fileName  # write the record to the file
      #echo "Inserted duplicate: $dupRecord"
      lineCount=$((lineCount + 1))  # increase the total record counter
      possibility=$((possibility + 2))  # possibility of creating another duplicate for the same citizenId is dropping to ~15%
    else
      break
    fi
  done
done
