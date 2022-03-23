typedef struct data *Data;

Data initializeData();
void deleteData(Data );
void readCitizenRecordsFile(Data ,char *,pthread_mutex_t *);
void vaccineStatusBloom(Data ,char *,char *);
char *vaccineStatus(Data ,char *,char *);
void vaccineStatusAll(Data ,char *,int );
void sendBloomFilters(int , Data );
