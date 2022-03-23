all:
	gcc -O3 -c countryLists/countryLists.c
	gcc -O3 -c errorChecking/errorChecking.c
	gcc -O3 -c hashTable/hashTable.c
	gcc -O3 -c bloomFilter/bloomFilter.c
	gcc -O3 -c -I hashTable -I countryLists skipList/skipList.c
	gcc -O3 -c -I bloomFilter -I skipList -I hashTable -I countryLists -I errorChecking database/database.c
	gcc -O3 -I database -I errorChecking -c main.c
	gcc -O3 -o vaccineMonitor main.o database.o skipList.o bloomFilter.o hashTable.o countryLists.o errorChecking.o

clean:
	rm -rf *.o vaccineMonitor
