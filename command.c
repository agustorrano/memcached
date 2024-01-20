#include "command.h"

void init_cache(Cache cache, int capacity, HashFunction hash) {
  cache->table = create_hashtable(capacity, hash);
  pthread_mutex_init(&cache->mutexTh, NULL);
  cache->textSt = create_stats();
  cache->binSt = create_stats();
  pthread_mutex_init(&cache->mutexTextSt, NULL);
  pthread_mutex_init(&cache->mutexBinSt, NULL);
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
  destroy_stats(cache->textSt);
  destroy_stats(cache->binSt);
  pthread_mutex_destroy(&cache->mutexTh);
  pthread_mutex_destroy(&cache->mutexTextSt);
  pthread_mutex_destroy(&cache->mutexBinSt);
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


void put(Cache cache, ConcurrentQueue queue, char *val, char *key, int mode)
{
  stats_nput(cache, mode);
  Data data = create_data(val, key, mode);
  insert_cache(cache, data);
  push_concurrent_queue(queue, key);
  return;
}

int del(Cache cache, ConcurrentQueue queue, char *key, int mode)
{
  stats_ndel(cache, mode);
  int i; // 1 si borro algo, 0 si no borro
  i = delete_in_cache(cache, key);
  if (i == 1)
    delete_in_concurrent_queue(queue, key);
  return i;
}

char *get(Cache cache, ConcurrentQueue queue, char *key, int mode)
{
  stats_nget(cache, mode);
  Data found = search_cache(cache, key);
  if (found == NULL) return NULL;
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

void get_stats(Cache cache, int fd, int mode)
{
  int nput, ndel, nget;
  if (mode == TEXT_MODE) {
    nput = cache->textSt->nput;
    nget = cache->textSt->nget;
    ndel = cache->textSt->ndel;
  } else {
    nput = cache->binSt->nput;
    nget = cache->binSt->nget;
    ndel = cache->binSt->ndel;
  }
  char buffer[2048];
  // Formatear el mensaje en el búfer
  snprintf(buffer, sizeof(buffer), "OK PUTS=%d DELS=%d GETS=%d KEYS=%d...\n",
    nput, ndel, nget, cache->table->numElems);
  if (write(fd, buffer, strlen(buffer)) < 0) {
    perror("Error al escribir en el socket");
    exit(EXIT_FAILURE);
  }
}

void destroy_stats(Stats stats) {
  free(stats);
  return;
}

void stats_nput(Cache cache, int mode) { 
  if (mode == TEXT_MODE) {
    pthread_mutex_lock(&cache->mutexTextSt);
    cache->textSt->nput++; 
    pthread_mutex_unlock(&cache->mutexTextSt);
  } else {
    pthread_mutex_lock(&cache->mutexBinSt);
    cache->binSt->nput++; 
    pthread_mutex_unlock(&cache->mutexBinSt);
  }
  return;
}

void stats_nget(Cache cache, int mode) {
  if (mode == TEXT_MODE) {
    pthread_mutex_lock(&cache->mutexTextSt);
    cache->textSt->nget++; 
    pthread_mutex_unlock(&cache->mutexTextSt);
  } else {
    pthread_mutex_lock(&cache->mutexBinSt);
    cache->binSt->nget++; 
    pthread_mutex_unlock(&cache->mutexBinSt);
  }
  return;
}

void stats_ndel(Cache cache, int mode) { 
  if (mode == TEXT_MODE) {
    pthread_mutex_lock(&cache->mutexTextSt);
    cache->textSt->ndel++; 
    pthread_mutex_unlock(&cache->mutexTextSt);
  } else {
    pthread_mutex_lock(&cache->mutexBinSt);
    cache->binSt->ndel++; 
    pthread_mutex_unlock(&cache->mutexBinSt);
  }
  return;
}