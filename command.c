#include "command.h"

void init_cache(Cache cache, int capacity, HashFunction hash) {
  cache->table = create_hashtable(capacity, hash);
  pthread_mutex_init(&cache->mutexTh, NULL);
  cache->stats = create_stats();
  pthread_mutex_init(&cache->mutexSt, NULL);
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
  destroy_stats(cache->stats);
  pthread_mutex_destroy(&cache->mutexTh);
  pthread_mutex_destroy(&cache->mutexSt);
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
  stats_nput(cache);
  Data data = create_data(val, key, mode);
  insert_cache(cache, data);
  push_concurrent_queue(queue, key);
  return;
}

int del(Cache cache, ConcurrentQueue queue, char *key)
{
  stats_ndel(cache);
  int i; // 1 si borro algo, 0 si no borro
  i = delete_in_cache(cache, key);
  if (i == 1)
    delete_in_concurrent_queue(queue, key);
  return i;
}

char *get(Cache cache, ConcurrentQueue queue, char *key)
{
  stats_nget(cache);
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

void get_stats(Cache cache, int fd)
{
  char buffer[2048];
  // Formatear el mensaje en el búfer
  snprintf(buffer, sizeof(buffer), "OK PUTS=%d DELS=%d GETS=%d KEYS=%d...\n",
    cache->stats->nput, cache->stats->ndel, cache->stats->nget, cache->table->numElems);
  if (write(fd, buffer, strlen(buffer)) < 0) {
    perror("Error al escribir en el socket");
    exit(EXIT_FAILURE);
  }
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