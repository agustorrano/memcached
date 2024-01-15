#include "command.h"

void init_cache(Cache cache, int capacity, HashFunction hash) {
  cache->table = create_hashtable(capacity, hash);
  pthread_mutex_init(&cache->mutexTh, NULL);
  cache->stats = create_stats();
  pthread_mutex_init(&cache->mutexSt, NULL);
  return;
}

void insert_cache(Cache cache, CData data) {
  pthread_mutex_lock(&cache->mutexTh);
  insert_hashtable(cache->table, data);
  pthread_mutex_unlock(&cache->mutexTh);
  return;
}

void destroy_cache(Cache cache) {
  destroy_hashtable(cache->table);
  destroy_stats(cache->stats);
  pthread_mutex_destroy(&cache->mutexTh);
  pthread_mutex_destroy(&cache->mutexSt);
  free(cache);
  return;
}

CData search_cache(Cache cache, char* key) {
  pthread_mutex_lock(&cache->mutexTh);
  CData data = search_hashtable(cache->table, key);
  pthread_mutex_unlock(&cache->mutexTh);
  return data;
}

void delete_in_cache(Cache cache, char* key, int *flag) {
  pthread_mutex_lock(&cache->mutexTh);
  delete_in_hashtable(cache->table, key, flag);
  pthread_mutex_unlock(&cache->mutexTh);
  return;
}


void put(Cache cache, ConcurrentQueue queue, char *val, char *key, int mode)
{
  stats_nput(cache);
  CData data = create_cdata(val, key, mode);
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
  CData found = search_cache(cache, key);
  if (found == NULL)
    return NULL;
  push_concurrent_queue(queue, key);
  return found->val;
}

Stats create_stats() {
  Stats stats = malloc(sizeof(struct _Stats));
  stats->nput = 0;
  stats->nget = 0;
  stats->ndel = 0;
  return stats;
}

void get_stats(Cache cache)
{
  printf("OK PUTS=%d DELS=%d GETS=%d KEYS=%d...\n", cache->stats->nput,
         cache->stats->ndel, cache->stats->nget, cache->table->numElems);
  return;
}

void destroy_stats(Stats stats) {
  free(stats);
  return;
}

void stats_nput(Cache cache) { 
  pthread_mutex_lock(&cache->mutexSt);
  cache->stats->nput++; 
  pthread_mutex_unlock(&cache->mutexSt);
  return;
}

void stats_nget(Cache cache) { 
  pthread_mutex_lock(&cache->mutexSt);
  cache->stats->nget++; 
  pthread_mutex_unlock(&cache->mutexSt);
  return;
}

void stats_ndel(Cache cache) { 
  pthread_mutex_lock(&cache->mutexSt);
  cache->stats->ndel++; 
  pthread_mutex_unlock(&cache->mutexSt);
  return;
}