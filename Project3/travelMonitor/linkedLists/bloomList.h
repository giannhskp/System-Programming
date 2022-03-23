#include "../bloomFilter/bloomFilter.h"
typedef struct bfn{
  char *virus;
  bloomFilter bf;
  struct bfn *next;
}bfListNode;
typedef bfListNode *bloomList;

bloomList initializeBloomList();
bloomList insertBloomToList(bloomList ,char *,bloomFilter );
bloomFilter getBloomFromList(bloomList ,char *);
void deleteBloomList(bloomList );
