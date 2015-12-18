#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

struct cacheEntry;

typedef struct cacheEntry {
  struct cacheEntry *next;
  struct cacheEntry *prev;
  int *address;
} cacheEntry;

typedef struct cacheSet {
  int maxCount;
  int entrySize;
  cacheEntry *first;
  cacheEntry *last;
} cacheSet;

bool inCache(char* addr, cacheSet cache){
  int i;
  int address = (int)strtol(addr, NULL, 16);
  cacheEntry* current = cache.first;
  cacheEntry* scratch;

  if (current == NULL){
    // will occur on first cache access attempt
    current = malloc(sizeof(cacheEntry));
    *current->address = address;
    return false;
  }

  for(i=0; i < (cache.maxCount); i++){
    if(*current->address >= address && *current->address < address+cache.entrySize ){
      return true;
      //move current* to head of list
      if (current->prev != NULL){
        current->prev->next = current->next;
        if (current->next != NULL){
          current->next->prev = current->prev;
        }
        else {
          cache.last = current->prev;
        }
        current->prev = NULL;
        current->next = cache.first;
        cache.first->prev = current;
        cache.first = current;
      }
    }
    else {
      if(current->next == NULL && i < cache.maxCount-1){
        // if we're not at at the end of the cache and the next element in the list is null
        // make a new cache entry and put it at the start of the list
        scratch = malloc(sizeof(cacheEntry));
        *scratch->address = address;
        scratch->next = cache.first;
        cache.first->prev = scratch;
        cache.first = scratch;
        // scoot back up on the list
        cache.last = cache.last->prev;
        if (cache.last != NULL){
          // if the something was ejected from the lru, free the memory and discard pointer to it
          free(cache.last->next);
          cache.last->next = NULL;
        }
        return false;
      }
      else {
        // we didn't match, move on to next element in cache
        current = current->next;
      }
    }
  }

  // didn't find address in cache, fetch it and move to front
  scratch = malloc(sizeof(cacheEntry));
  *scratch->address = address;
  scratch->next = cache.first;
  cache.first->prev = scratch;
  cache.first = scratch;
  // scoot back up on the list
  cache.last = cache.last->prev;
  free(cache.last->next);
  cache.last->next = NULL;

  return false;
}

int main(int argc, char *argv[]) {

  if (argc != 5){
    printf("Usage: ./cache L K N addresses.txt\n");
    return -1;
  }
  else{
    int l,k,n;
    char* addrfile;
    l = (int) strtol(argv[1], (char **)NULL, 10);
    k = (int) strtol(argv[2], (char **)NULL, 10);
    n = (int) strtol(argv[3], (char **)NULL, 10);
    if ( l <= 0 || k <= 0 || n <= 0 ){
      printf("Be sensible pls\n");
      return -1;
    }

    printf("%d %d %d\n", l, k, n);
  }
  return 0;
}
