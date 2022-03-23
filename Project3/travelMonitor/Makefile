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
	gcc -g -O3 -I bloomFilter -c socketReadWrite.c
	gcc -g -O3 -I bloomFilter -c travelMonitorFunctions.c
	gcc -g -O3 -I database -c readDirs.c
	gcc -g -O3 -I database -c readFiles.c
	gcc -g -O3 -I database -c monitorServer.c
	gcc -g -O3 -I database -c travelMonitorClient.c
	gcc -g -O3 -c threadFunctions.c -pthread
	gcc -g -pthread -o monitorServer monitorServer.o	readFiles.o stringList.o	database.o skipList.o bloomFilter.o hashTable.o countryLists.o errorChecking.o bloomList.o socketReadWrite.o threadFunctions.o
	gcc -g -o travelMonitorClient travelMonitorClient.o readDirs.o stringList.o	database.o skipList.o bloomFilter.o hashTable.o countryLists.o errorChecking.o bloomList.o socketReadWrite.o travelMonitorFunctions.o requestList.o

execute:
	./travelMonitor

clean:
	rm *.o
	rm travelMonitorClient monitorServer
