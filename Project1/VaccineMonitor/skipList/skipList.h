typedef struct sl *skipList;

skipList initializeSL();
int insertSL(skipList ,char *,char *,citizenRecord );
int searchSL(skipList ,char *);
char  *searchGetDateSL(skipList ,char *);
void deleteSL(skipList ,char *);
void destroySL(skipList );
void printSL(skipList );

int getRecords(skipList );
void listPersons(skipList );
countryCounterList populationStatusSL(countryCounterList ,skipList ,char *,char *,char *,int );
countryAgeList popStatusByAgeSL(countryAgeList ,skipList ,char *,char *,char *,int );
