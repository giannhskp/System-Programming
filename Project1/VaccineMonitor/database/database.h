typedef struct data *Data;

Data initializeData();
void deleteData(Data );
void readCitizenRecordsFile(Data ,char *);
void vaccineStatusBloom(Data ,char *,char *);
void vaccineStatus(Data ,char *,char *);
void vaccineStatusAll(Data ,char *);
void insertCitizenRecord(Data ,char *,char *,char *,char *,int ,char *,char *,char *);
void vaccinateNow(Data ,char *,char *,char *,char *,int ,char *);
void listNonVaccinatedPersons(Data ,char *);
void populationStatus(Data ,char *,char *,char *,char *);
void popStatusByAge(Data ,char *,char *,char *,char *);

int validDate(const char[]);
