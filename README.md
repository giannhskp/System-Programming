# System-Programming

## Project 1 : Vaccination App

### Part 1 : Vaccination App
Developed an app that receives, processes, record and answers questions on whether a citizent is vaccinated or not.
The basic goal of the project is that the app uses the following datastractures in order to handle the data and operate on question answering:
  - [Bloom Filter](https://en.wikipedia.org/wiki/Bloom_filter)
  - [Skip List](https://en.wikipedia.org/wiki/Skip_list)
  - Hash Tables, Linked Lists, etc.

### Part 2 : Bash Script
Created a bash script that generates test files for the Vaccination App (Part 1).
The script takes as input:
  - virusesFile: a file with viruses names (one per line)
  - countriesFile: a file with countries names (one per line)
  - numLines: number of lines of the output file that will be generated
  - duplicatesAllowed: if 0, citizenIDs must be unique, if 1 dulpicate citizenIDs are allowed

## Project 2 : Travel Monitor
Goal of this project is to familiarize with:
  - Processes creation using system calls (fork/exec)
  - Process communication via pipes
  - Use of low level I/O
  - Bash Script creation

### Part 1 : Travel Monitor App
Developed an app that uses:
  - Data Structures of Project 1 (Bloom Filter, Skip List)
  - Processes creation using system calls (fork/exec)
  - Communication between multiple processes using pipes
  - Low level I/O (e.g. Signals)
In order to execute the following operations:
  - Decide whether a citizen can travel to a country at a given date, taking into considaration his vaccination information.
  - Add new vaccination info to the "database" (e.g. if a citizen has just vaccinated)
  - Find the vaccination status of a given citizen.
  - Compute and print statistics about the travel requests that have been made.

### Part 2 : Bash Script
Created a bash script that generates test subdirectories and input files for the Travel Monitor App (Part 1).
The script takes as input:
  - inputFile: a file with same format as the input files of Project 1.
  - input_dir: the name of the directories where the subdirectories will be placed.
  - numFilesPerDirectory: number of files tha every directory will contain.
