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

More info on the project description on [hw2-spring-2021.pdf](https://github.com/giannhskp/System-Programming/blob/main/Project2/hw2-spring-2021.pdf) file.

More info on the project implementation on [README.txt](https://github.com/giannhskp/System-Programming/blob/main/Project2/README.txt) file.
