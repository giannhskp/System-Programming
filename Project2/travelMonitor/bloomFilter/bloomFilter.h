#include <stdint.h>
typedef struct bfnode *bloomFilter;
typedef uint8_t byte;

bloomFilter createBloom(int );
byte *getBloomArray(bloomFilter );
bloomFilter setBloom(byte * ,int );
void bloomInsert(bloomFilter ,char *);
int bloomExists(bloomFilter ,char *);
void deleteBloom(bloomFilter );
