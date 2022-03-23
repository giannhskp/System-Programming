#!/bin/bash

if (($#!=3)); then  #check if the correct number of arguments was given
  echo "Wrong input"
  exit
fi
inputFile=$1
input_dir=$2
numFilesPerDirectory=$3
if [ ! -e "$inputFile" ]; then  # check if the input file actually exists
  echo "Error: $inputFile does not exist"
  exit
fi
if [ -d "$input_dir" ]; then  # check if the output directory already exists
  echo "Error: $input_dir directory already exist"
  exit
fi
if [[ ${numFilesPerDirectory} =~ [a-zA-Z] ]] ; then   # check if the numFilesPerDirectory is actually an integer
  echo "Wrong input: numFilesPerDirectory not a number"
  exit
fi
if [ $numFilesPerDirectory -le 0 ]; then  # check if the numFilesPerDirectory is a positive number
  echo "Wrong input: numFilesPerDirectory needs to be a positive number"
  exit
fi

mkdir $input_dir  # create the output directory
countries=()  # array that will contain all the countries of the input file
rrCounter=()  # array that will contain in which of the country-n.txt files the next record should be written
# each index of the array corresponds to a country that exists in the same index in the countries array

while read -r line; do  # read the records of the input file
  tempArr=($line) # convert the record to an array of words
  country=${tempArr[3]} # take only the country of the record
  if [[ ! "${countries[@]}" =~ "${country}" ]]; then  # if the country does not exist in the countries array
    countries+=($country) # add the country to the arrays
    rrCounter+=(0)  # initialize the file counter in order to start writing to the first file of the country
  fi
done < $inputFile

for country in "${countries[@]}"; do  # create the country directories and the country files inside those directories
  # for every country
  path="${input_dir}/${country}"  # create the path of the certain country directory
  mkdir $path # create the directory
  for (( i=1; i <= $numFilesPerDirectory; i++ )); do  # create numFilesPerDirectory files for this country, inside the directory
    filePath="${path}/${country}-${i}.txt"  # create the path of the file
    touch $filePath # create an empty file
  done
done

while read -r record; do  # for every record of the file
  tempArr=($record)
  country=${tempArr[3]} # get the country of the record
  # find in which of the numFilesPerDirectory files of the country the new record should be written
  for(( i=0; i<${#countries[@]}; i++ )); do
    # find the index of the country in the countries array
    if [[ "${countries[${i}]}" == "${country}" ]]; then
      # we found the index of the array
      roundRobinCounter=${rrCounter[$i]}  # get from the rrCounter array in which file should we write
      fileNum=$(($roundRobinCounter + 1))
      filePath="${input_dir}/${country}/${country}-${fileNum}.txt" # create the path of the file
      echo $record >> $filePath # write the record to that file
      # increase the counter in order to write the next record of this country to the next file
      rrCounter[$i]=$(( (${roundRobinCounter}+1) % $numFilesPerDirectory ))
      break
    fi
  done
done < $inputFile
