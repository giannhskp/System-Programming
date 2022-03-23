typedef struct cl *countryCounterList;

countryCounterList initializeCCL();
countryCounterList increaseCounter(countryCounterList ,char *,int );
void printCCL(countryCounterList );
void deleteCCL(countryCounterList );

typedef struct al *countryAgeList;

countryAgeList initializeCAL();
countryAgeList increaseAgeCounter(countryAgeList ,char *,int ,int );
void printCAL(countryAgeList );
void deleteCAL(countryAgeList );

typedef struct cn *countryList;
countryList initializeCountryList();
countryList insertGetCountry(countryList ,char *,char **);
void deleteCountryList(countryList );
