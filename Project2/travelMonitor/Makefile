all:
	gcc -g -O3 -c linkedLists/countryLists.c
	gcc -g -O3 -c errorChecking/errorChecking.c
	gcc -g -O3 -c hashTable/hashTable.c
	gcc -g -O3 -c bloomFilter/bloomFilter.c
	gcc -g -O3 -c -I hashTable -I countryLists skipList/skipList.c
	gcc -g -O3 -c -I bloomFilter -I skipList -I hashTable -I countryLists -I errorChecking database/database.c
	gcc -g -O3 -c linkedLists/stringList.c
	gcc -g -O3 -c linkedLists/requestList.c
	gcc -g -O3 -I bloomFilter -c linkedLists/bloomList.c
	gcc -g -O3 -I bloomFilter -c pipeReadWrite.c
	gcc -g -O3 -I bloomFilter -c travelMonitorFunctions.c
	gcc -g -O3 -I database -c readDirs.c
	gcc -g -O3 -I database -c monitor.c
	gcc -g -O3 -I database -c travelMonitor.c
	gcc -g -o travelMonitor travelMonitor.o readDirs.o stringList.o	database.o skipList.o bloomFilter.o hashTable.o countryLists.o errorChecking.o bloomList.o pipeReadWrite.o travelMonitorFunctions.o requestList.o
	gcc -g -o Monitor monitor.o	readDirs.o stringList.o	database.o skipList.o bloomFilter.o hashTable.o countryLists.o errorChecking.o bloomList.o pipeReadWrite.o

execute:
	./travelMonitor

clean:
	rm *.o
