#include "command.h"

Cache cache;
Stats* statsTh;

void lock_cache(Cache cache) {
  pthread_mutex_lock(&cache->mutexTh);
}

void unlock_cache(Cache cache) {
  pthread_mutex_unlock(&cache->mutexTh);
}

HashTable table_cache(Cache cache) {
  return cache->table;
}

void init_cache(Cache cache, ConcurrentQueue queue, int capacity, HashFunction hash) {
  cache->table = create_hashtable(capacity, hash);
  config_mutex(&cache->mutexTh);
  init_concurrent_queue(queue);
  cache->queue = queue;
  return;
}

void destroy_cache(Cache cache) {
  destroy_hashtable(cache->table);
  pthread_mutex_destroy(&cache->mutexTh);
  destroy_concurrent_queue(cache->queue);
  free(cache);
  return;
}

enum code put(Cache cache, Stats stats, char *val, char *key, int mode, int vlen, int klen)
{
  stats_nput(stats);
  lock_cache(cache);
  HashTable table = table_cache(cache);
  unsigned idx = idx_hashtable(table, key);
  DNode* node = search_list(table->elems[idx], key);
  if (node == NULL) {
    char* keyCpy, *valCpy;
    if (try_malloc(klen, (void*)&keyCpy) == -1) {
      unlock_cache(cache);
      return EOOM;
    } 
    else if (try_malloc(vlen, (void*)&valCpy) == -1) {
      free(keyCpy);
      unlock_cache(cache);
      return EOOM;
    }
    memcpy(keyCpy, key, klen);
    memcpy(valCpy, val, vlen);
    Data data = create_data(valCpy, keyCpy, mode, vlen);
    if (data == NULL) {
      free(keyCpy);
      free(valCpy);
      unlock_cache(cache);
      return EOOM;
    }
    int flag_enomem = 0;
    DNode* node = insert_beginning_list(table->elems[idx], data, &flag_enomem);
    if (flag_enomem == 0) {
      push_queue(cache->queue, data, &flag_enomem);
      if (flag_enomem == 1) {
        destroy_data(data);
        free(node);
        unlock_cache(cache);
        return EOOM;
      }
    }
    else {
      destroy_data(data);
      unlock_cache(cache);
      return EOOM;
    }
  }
  else {
    char* valCpy;
    Data data = node->data;
    if (try_malloc(vlen, (void*)valCpy) == -1) {
      unlock_cache(cache);
      return EOOM;
    }
    memcpy(valCpy, val, vlen);
    free(data->val);
    data->val = valCpy;
    data->vlen = vlen;
    node->data = data;
    update_concurrent_queue(cache->queue, node);
  }
  unlock_cache(cache);
  return OK;
}

enum code del(Cache cache, Stats stats, char *key)
{
  stats_ndel(stats);
  lock_cache(cache);
  HashTable table = cache->table;
  DNode* node = delete_in_hashtable(table, key);
  if (node == NULL) {
    unlock_cache(cache);
    return ENOTFOUND;
  }
  delete_in_concurrent_queue(cache->queue, node);
  destroy_data(node->data);
  free(node);
  unlock_cache(cache);
  return OK;
}

enum code get(Cache cache, Stats stats, int mode, char *key, char** val, int* vlen)
{
  stats_nget(stats);
  lock_cache(cache);
  HashTable table = table_cache(cache);
  unsigned idx = idx_hashtable(table, key);
  DNode* node = search_list(table->elems[idx], key);
  if (node == NULL) {
    unlock_cache(cache);
    return ENOTFOUND;
  }
  Data data = node->data;
  if (mode == TEXT_MODE && data->mode == BIN_MODE) {
    unlock_cache(cache);
    return EBINARY;
  }
  *vlen = data->vlen;
  if (try_malloc(sizeof(char*) * (*vlen), (void*)val) == -1) {
    unlock_cache(cache);
    return EOOM;
  }
  memcpy(*val, data->val, *vlen);
  update_concurrent_queue(cache->queue, node);
  unlock_cache(cache);
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

