typedef struct cn{
  char *string;
  struct cn *next;
}node;
typedef node *stringList;

stringList initializeStringList();
stringList insertString(stringList ,char *);
int stringExists(stringList ,char *);
void deleteStringList(stringList );
