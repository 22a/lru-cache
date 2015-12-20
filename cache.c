#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

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

bool inCache(char* addr, cacheSet* cache){
  int i;
  int address = (int)strtol(addr, NULL, 16);
  printf("address: %d\n", address);
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
        if (cache->last){
          // scoot back up on the list
          cache->last = cache->last->prev;
          free(cache->last->next);
          cache->last->next = NULL;
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
  while(current){
    printf("%d, ",current->address);
    current = current->next;
  }
  printf("\n");
}

int main(int argc, char *argv[]) {

  if (argc != 5){
    printf("Usage: ./cache L K N addresses.txt\n");
    return -1;
  }
  else{
    int l,k,n,i;
    char* addrfile;
    l = (int) strtol(argv[1], NULL, 10);
    k = (int) strtol(argv[2], NULL, 10);
    n = (int) strtol(argv[3], NULL, 10);
    if ( l <= 0 || k <= 0 || n <= 0 ){
      printf("Be sensible pls\n");
      return -1;
    }
    printf("L: %d, K: %d, N: %d\n", l, k, n);

    cacheSet** cache = calloc(k,sizeof(cacheSet*));
    for(i=0; i < k; i++){
      cache[i] = newCacheSet(n,l);
    }

    char* testAddrs[] = {"1000","1000","1004","1010","100C","1000"};

    for(i=0; i < 6; i++){
      printCacheSetTags(cache[0]);
      bool b = inCache(testAddrs[i],cache[0]);
      printf(b ? "true\n" : "false\n");
    }
  }
  return 0;
}
