#include "command.h"

Cache cache;
Stats* statsTh;

void init_cache(Cache cache, ConcurrentQueue queue, int capacity, HashFunction hash) {
  cache->table = create_hashtable(capacity, hash);
  pthread_mutex_init(&cache->mutexTh, NULL);
  init_concurrent_queue(queue);
  cache->queue = queue;
  return;
}

void insert_cache(Cache cache, Data data) {
  pthread_mutex_lock(&cache->mutexTh);
  insert_hashtable(cache->table, data);
  pthread_mutex_unlock(&cache->mutexTh);
  return;
}

void destroy_cache(Cache cache) {
  destroy_hashtable(cache->table);
  pthread_mutex_destroy(&cache->mutexTh);
  destroy_concurrent_queue(cache->queue);
  free(cache);
  return;
}

Data search_cache(Cache cache, char* key) {
  pthread_mutex_lock(&cache->mutexTh);
  Data data = search_hashtable(cache->table, key);
  pthread_mutex_unlock(&cache->mutexTh);
  return data;
}

int delete_in_cache(Cache cache, char* key) {
  pthread_mutex_lock(&cache->mutexTh);
  int i = delete_in_hashtable(cache->table, key);
  pthread_mutex_unlock(&cache->mutexTh);
  return i;
}

enum code put(Cache cache, Stats stats, char *val, char *key, int mode, int vlen)
{
  stats_nput(stats);
  Data data = create_data(val, key, mode, vlen);
  insert_cache(cache, data);
  push_concurrent_queue(cache->queue, key);
  return OK;
}

enum code del(Cache cache, Stats stats, char *key)
{
  stats_ndel(stats);
  if (delete_in_cache(cache, key)) {
    delete_in_concurrent_queue(cache->queue, key);
    return OK;
  }
  return ENOTFOUND;
}

enum code get(Cache cache, Stats stats, int mode, char *key, char** val, int* vlen)
{
  stats_nget(stats);
  Data found = search_cache(cache, key);
  if (found == NULL) 
    return ENOTFOUND;
  if (mode == TEXT_MODE && found->mode == BIN_MODE)
    return EBINARY;
  push_concurrent_queue(cache->queue, key);

  *vlen = found->vlen;
  //*val = malloc(*vlen);
  //log(1, "strlen: %d", *vlen);
  try_malloc(sizeof(int)*(*vlen), (void*)val);
  memcpy(*val, found->val, *vlen);
  return OK;
}

Stats create_stats() {
  Stats stats;
  try_malloc(sizeof(struct _Stats), (void*)&stats);
  //Stats stats = malloc(sizeof(struct _Stats));
  stats->nput = 0;
  stats->nget = 0;
  stats->ndel = 0;
  pthread_mutex_init(&stats->mutexSt, NULL);
  return stats;
}

enum code get_stats(Stats* stats, Stats allStats)
{
  for (int i = 0; i < numofthreads; i++) {
    pthread_mutex_lock(&stats[i]->mutexSt);
    allStats->nput += stats[i]->nput;
    allStats->nget += stats[i]->nget;
    allStats->ndel += stats[i]->ndel;
    pthread_mutex_unlock(&stats[i]->mutexSt);
  }
  return OK;
}

int print_stats(Cache cache, Stats stats, char** res) {
  //*res = malloc(MAX_BUF_SIZE);
  try_malloc(sizeof(char)*MAX_BUF_SIZE, (void*)res);
  // Formatear el mensaje en el búfer
  int len = snprintf(*res, MAX_BUF_SIZE, "PUTS=%d DELS=%d GETS=%d KEYS=%d...",
    stats->nput, stats->ndel, stats->nget, cache->table->numElems);
  return len;
}

void destroy_stats(Stats stats) {
  pthread_mutex_destroy(&stats->mutexSt);
  free(stats);
  return;
}

void stats_nput(Stats stats) { 
  pthread_mutex_lock(&stats->mutexSt);
  stats->nput++; 
  pthread_mutex_unlock(&stats->mutexSt);
  return;
}

void stats_nget(Stats stats) {
  pthread_mutex_lock(&stats->mutexSt);
  stats->nget++; 
  pthread_mutex_unlock(&stats->mutexSt);
  return;
}

void stats_ndel(Stats stats) { 
  pthread_mutex_lock(&stats->mutexSt);
  stats->ndel++; 
  pthread_mutex_unlock(&stats->mutexSt);
  return;
}

