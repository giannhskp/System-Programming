typedef struct bfnode *bloomFilter;

bloomFilter createBloom(int );
void bloomInsert(bloomFilter ,char *);
void bloomExists(bloomFilter ,char *);
void deleteBloom(bloomFilter );
