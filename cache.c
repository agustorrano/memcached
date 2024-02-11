#include "cache.h"

int idx_mutex(unsigned idx) {
  int idxMutex = idx / NUM_BLOCKS;
  return idxMutex;
}

void init_cache(Cache cache, ConcurrentQueue queue, int capacity, HashFunction hash) {
  cache->table = create_hashtable(capacity, hash);
  for (int i = 0; i < NUM_MUTEX; i++)
    config_mutex(&cache->mutexTh[i]);
  init_concurrent_queue(queue);
  cache->queue = queue;
  return;
}


void destroy_cache(Cache cache) {
  destroy_hashtable(cache->table);
  for (int i = 0; i < NUM_MUTEX; i++)
    pthread_mutex_destroy(&cache->mutexTh[i]);
  destroy_concurrent_queue(cache->queue);
  free(cache);
  return;
}

int delete_in_cache(Cache cache, char* key, int idxMutex) {
  if (key == NULL) {return 0;}
  pthread_mutex_lock(&cache->mutexTh[idxMutex]);
  int i = delete_in_hashtable(cache->table, key);
  pthread_mutex_unlock(&cache->mutexTh[idxMutex]);
  return i;
}

int lock_cache(Cache cache, char* key) {
  unsigned idx = idx_hashtable(cache->table, key);
  int idxMutex = idx_mutex(idx);
  pthread_mutex_lock(&cache->mutexTh[idxMutex]);
  return idxMutex;
}

void unlock_cache(Cache cache, int idxMutex) {
  pthread_mutex_unlock(&cache->mutexTh[idxMutex]);
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

void destroy_stats(Stats stats) {
  pthread_mutex_destroy(&stats->mutexSt);
  free(stats);
  return;
}