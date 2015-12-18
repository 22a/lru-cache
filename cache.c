#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

struct cacheEntry;

typedef struct cacheEntry {
  struct cacheEntry *next;
  struct cacheEntry *prev;
  int *address;
} cacheEntry;

typedef struct lruCache {
  int maxCount;
  cacheEntry *first;
  cacheEntry *last;
} lruCache;

bool inCache(char* addr, lruCache cache){
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
    if(*current->address >= address && *current->address < address+16 ){
      return true;
      //move current* to head of list
      if (current->prev != NULL){
        current->prev->next = current->next;
        if (current->next != NULL){
          current->next->prev = current->prev;
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
      }
      else {
        // we didn't match, move on to next element in cache
        current = current->next;
      }
    }
  }
  return false;
}

int main(int argc, char *argv[]) {
  printf("hello world\n");


  return 0;

}
