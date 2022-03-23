typedef struct hash *HashTable;
typedef struct node *citizenRecord;

char *getId(citizenRecord );
char *getFirstName(citizenRecord );
char *getLastName(citizenRecord );
char *getCountry(citizenRecord );
int getAge(citizenRecord );

HashTable htCreate(int );
citizenRecord htSearchOrInsert(HashTable ,char *,char *,char *,char *,int );
citizenRecord htSearch(HashTable ,char *);
void htDestroy(HashTable );
