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

void insert_cache(Cache cache, Data data, int* flag_enomem) {
  pthread_mutex_lock(&cache->mutexTh);
  insert_hashtable(cache->table, data, flag_enomem);
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
  if (data == NULL) return EOOM;
  int flag_enomem = 0;
  insert_cache(cache, data, &flag_enomem);
  free(data);
  if (flag_enomem) return EOOM;
  push_concurrent_queue(cache->queue, key, &flag_enomem);
  if (flag_enomem) return EOOM;
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
  int flag_enomem = 0;
  push_concurrent_queue(cache->queue, key, &flag_enomem);
  if (flag_enomem) return EOOM;
  *vlen = found->vlen;
  if (try_malloc(sizeof(int)*(*vlen), (void*)val) == -1) return EOOM;
  //log(1, "found->val: %s\t veln: %d", found->val, *vlen);
  memcpy(*val, found->val, *vlen);
  return OK;
}

Stats create_stats() {
  Stats stats;
  if (try_malloc(sizeof(struct _Stats), (void*)&stats) == -1) { return NULL; }
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

uint64_t get_numElems_concurrent(Cache cache) {
  pthread_mutex_lock(&cache->mutexTh);
  uint64_t numKeys = cache->table->numElems;
  pthread_mutex_unlock(&cache->mutexTh);
  return numKeys;
}

int print_stats(Cache cache, Stats stats, char** res) {
  if (try_malloc(sizeof(char)*MAX_BUF_SIZE, (void*)res) == -1) { return -1; }
  // Formatear el mensaje en el búfer
  uint64_t numKeys = get_numElems_concurrent(cache); 
  int len = snprintf(*res, MAX_BUF_SIZE, "PUTS=%ld DELS=%ld GETS=%ld KEYS=%ld",
    stats->nput, stats->ndel, stats->nget, numKeys);
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

