#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

struct cacheEntry;

typedef struct cacheEntry {
  struct cacheEntry *next;
  struct cacheEntry *prev;
  int address;
} cacheEntry;

cacheEntry* newCacheEntry(int addr, cacheEntry* prev, cacheEntry* next){
  cacheEntry* new = malloc(sizeof(cacheEntry));
  new -> address = addr;
  new -> prev = prev;
  new -> next = next;
  return new;
}

typedef struct cacheSet{
  int maxCount;
  int count;
  int entrySize;
  cacheEntry *first;
  cacheEntry *last;
} cacheSet;

cacheSet* newCacheSet(int maxCount, int size){
  cacheSet* new = malloc(sizeof(cacheSet));
  new -> maxCount = maxCount;
  new -> count = 0;
  new -> entrySize = size;
  new -> first = NULL;
  new -> last = NULL;
  return new;
}

bool inCache(int address, cacheSet* cache){
  int i;
  cacheEntry* current = cache->first;
  cacheEntry* scratch;

  if (!current){
    // will occur on first cache access attempt
    cache->first = newCacheEntry(address-address%cache->entrySize,NULL,NULL);
    cache->count++;
    if(cache->count == cache->maxCount){
      // if the cache is one way
      cache->last = cache->first;
    }
    return false;
  }

  for(i=0; i < (cache->maxCount); i++){
    if(current->address <= address && address < current->address+cache->entrySize ){
      //move current* to head of list
      if (current->prev != NULL){
        current->prev->next = current->next;
        if (current->next != NULL){
          current->next->prev = current->prev;
        }
        else {
          cache->last = current->prev;
        }
        current->prev = NULL;
        current->next = cache->first;
        cache->first->prev = current;
        cache->first = current;
      }
      return true;
    }
    else {
      if(current->next == NULL && i < cache->maxCount-1){
        // if we're not at at the end of the cache and the next element in the list is null
        // make a new cache entry and put it at the start of the list
        scratch = newCacheEntry(address,NULL,cache->first);
        cache->first->prev = scratch;
        cache->first = scratch;

        cache->count++;
        if(cache->count == cache->maxCount){
          // having just populated the last free slot in the set update the cacheSet object
          scratch = cache->first;
          while(scratch){
            if (!scratch->next){
              cache->last = scratch;
            }
            scratch = scratch->next;
          }
        }
        return false;
      }
      else {
        // we didn't match, move on to next element in cache
        if (current->next){
          current = current->next;
        }
        else {
          scratch = newCacheEntry(address-address%cache->entrySize,NULL,cache->first);
          cache->first->prev = scratch;
          cache->first = scratch;
          if (cache->last){
            // scoot back up on the list
            cache->last = cache->last->prev;
            free(cache->last->next);
            cache->last->next = NULL;
          }

          return false;
        }
      }
    }
  }

  // didn't find address in cache, fetch it and move to front
  scratch = newCacheEntry(address-address%cache->entrySize,NULL,cache->first);
  cache->first->prev = scratch;
  cache->first = scratch;
  // scoot back up on the list //TODO: assert that cache.last != NULL
  if (cache->last){
    cache->last = cache->last->prev;
    free(cache->last->next);
    cache->last->next = NULL;
  }

  return false;
}

void printCacheSetTags(cacheSet* cache){
  cacheEntry* current = cache->first;
  printf("contents of set: ");
  while(current){
    printf("%d, ",current->address);
    current = current->next;
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
      printf(b ? "\thit\n" : "\tmiss\n");
    }
    printf("\ntotal = %d\n", total);
    fclose(file);

  }
  return 0;
}
