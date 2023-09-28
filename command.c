#include "command.h"

void put(Cache cache, ConcurrentQueue queue, char *val, char *key)
{
  stats_nput(cache);
  Data data = create_data(val, key);
  insert_cache(cache, data);
  push_concurrent_queue(queue, key);
  return;
}

void del(Cache cache, ConcurrentQueue queue, char *key)
{
  stats_ndel(cache);
  int *flag = malloc(sizeof(int));
  *flag = 0;
  delete_in_cache(cache, key, flag);
  if (*flag == 1)
    delete_in_concurrent_queue(queue, key);
  free(flag);
  return;
}

char *get(Cache cache, ConcurrentQueue queue, char *key)
{
  stats_nget(cache);
  Data found = search_cache(cache, key);
  if (found == NULL)
    return NULL;
  push_concurrent_queue(queue, key);
  return found->val;
}

void get_stats(Cache cache)
{
  printf("OK PUTS=%d DELS=%d GETS=%d KEYS=%d...\n", cache->stats->nput,
         cache->stats->ndel, cache->stats->nget, cache->table->numElems);
  return;
}
