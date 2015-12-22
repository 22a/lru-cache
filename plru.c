#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

struct cacheEntry;

typedef struct cacheEntry {
  int address;
  bool mru_bit;
} cacheEntry;

cacheEntry* newCacheEntry(int addr){
  cacheEntry* new = malloc(sizeof(cacheEntry));
  new -> address = addr;
  new -> mru_bit = false;
  return new;
}

typedef struct cacheSet{
  int entrySize;
  int size;
  int mru_set;
  cacheEntry** line;
} cacheSet;

cacheSet* newCacheSet(int size, int offsetSize){
  cacheSet* new = malloc(sizeof(cacheSet));
  new -> entrySize = offsetSize;
  new -> size = size;
  new -> mru_set = 0;
  new -> line = calloc(size, sizeof(cacheEntry*));
  return new;
}

bool inCache(int address, cacheSet* cache){
  int i;

  for(i=0; i < (cache->size); i++){
    if(!cache->line[i]) {
      break;
    }
    if(cache->line[i]->address <= address && address < cache->line[i]->address+cache->entrySize ){
      if(!cache->line[i]->mru_bit){
        cache->line[i]->mru_bit = true;
        cache->mru_set++;
        if(cache->mru_set == cache->size) {
          for(i=0; i < cache->size; i++) {
            cache->line[i]->mru_bit = false;
            cache->mru_set = 0;
          }
        }
      }
      return true;
    }
  }
  // got to end of cacheline without hit

  for(i=0; i < cache->size; i++){
    if(!cache->line[i] || !cache->line[i]->mru_bit){
      cache->line[i] = newCacheEntry(address-address%cache->entrySize);
      cache->line[i]->mru_bit = true;
      cache->mru_set++;
      if(cache->mru_set == cache->size) {
        for(i=0; i < cache->size; i++) {
          cache->line[i]->mru_bit = false;
          cache->mru_set = 0;
        }
      }
      return false;
    }
  }
  //if flow control gets here something terrible has happened
  fprintf(stderr, "\n\n\nFATAL ERROR IN PLRU Replacement Policy\n\n\n");
  return false;
}


void printCacheSetTags(cacheSet* cache){
  int i = 0;
  printf("contents of set:");
  while(i < cache->size && cache->line[i]){
    printf(" %d|%d,", cache->line[i]->mru_bit, cache->line[i]->address);
    i++;
  }
}

int main(int argc, char *argv[]) {

  if (argc != 5){
    printf("Usage: ./cache L K N addresses.txt\n");
    return -1;
  }
  else{
    int l,k,n,i;
    char line[256];
    l = (int) strtol(argv[1], NULL, 10);
    k = (int) strtol(argv[2], NULL, 10);
    n = (int) strtol(argv[3], NULL, 10);
    if ( l <= 0 || k <= 0 || n <= 0 ){
      printf("Be sensible pls\n");
      return -1;
    }

    char const* const fileName = argv[4];
    FILE* file = fopen(fileName, "r");
    if (!file){
      printf("Invalid address file, could not open %s\n", argv[4]);
      return -1;
    }
    printf("L: %d, K: %d, N: %d\n", l, k, n);

    cacheSet** cache = calloc(n,sizeof(cacheSet*));
    for(i=0; i < n; i++){
      cache[i] = newCacheSet(k,l);
    }

    int offsetBits = (int)ceil(log(l)/log(2));
    int setBits = (int)ceil(log(n)/log(2));
    printf("log2(l) = %d\n", offsetBits);
    printf("log2(n) = %d\n", setBits);

    unsigned mask = 0;
    for(i=0;i<setBits;i++){
      mask = mask << 1;
      mask++;
    }

    unsigned set;
    int address;
    int total = 0;
    while (fgets(line, sizeof(line), file)) {
      address = (int)strtol(line, NULL, 16);
      printf("address: %d,\t",address);
      set = address >> offsetBits;
      set = set & mask;
      printf("set: %d,\t",(int) set);
      printCacheSetTags(cache[set]);
      bool b = inCache(address,cache[set]);
      total += (int)b;
      printf(b ? "\thit\n" : "\t\033[31;1mmiss\033[0m\n");
    }
    printf("\ntotal = %d\n", total);
    fclose(file);

  }
  return 0;
}
