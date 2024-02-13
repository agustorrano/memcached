#include "command.h"

Cache cache;
Stats *statsTh;

int lock_cache(Cache cache, char *key)
{
  unsigned idx = idx_hashtable(cache->table, key);
  int idxMutex = idx_mutex(idx);
  pthread_mutex_lock(&cache->mutexTh[idxMutex]);
  return idxMutex;
}

void unlock_cache(Cache cache, int idxMutex)
{
  pthread_mutex_unlock(&cache->mutexTh[idxMutex]);
}

int idx_mutex(unsigned idx)
{
  int idxMutex = idx / NUM_BLOCKS;
  return idxMutex;
}

void init_cache(Cache cache, ConcurrentQueue queue, unsigned capacity, HashFunction hash)
{
  cache->table = create_hashtable(capacity, hash);
  for (int i = 0; i < NUM_MUTEX; i++)
    config_mutex(&cache->mutexTh[i]);
  init_concurrent_queue(queue);
  cache->queue = queue;
  return;
}

void destroy_cache(Cache cache)
{
  destroy_hashtable(cache->table);
  for (int i = 0; i < NUM_MUTEX; i++)
    pthread_mutex_destroy(&cache->mutexTh[i]);
  destroy_concurrent_queue(cache->queue);
  free(cache);
  return;
}

int delete_in_cache(Cache cache, char *key, int idxMutex)
{
  if (key == NULL)
  {
    return 0;
  }
  pthread_mutex_lock(&cache->mutexTh[idxMutex]);
  int i = delete_in_hashtable(cache->table, key);
  pthread_mutex_unlock(&cache->mutexTh[idxMutex]);
  return i;
}

enum code put(Cache cache, Stats stats, char *val, char *key, int mode, unsigned int vlen)
{
  stats_nput(stats);
  Data data = create_data(val, key, mode, vlen);
  if (data == NULL)
  {
    return EOOM;
  }
  int idxMutex = lock_cache(cache, key);
  int flag_enomem = 0;
  insert_hashtable(cache->table, data, &flag_enomem);
  free(data);
  if (flag_enomem)
  {
    unlock_cache(cache, idxMutex);
    return EOOM;
  }
  ConcurrentQueue concq = cache->queue;
  update_queue(concq, key, &flag_enomem);
  if (flag_enomem)
  {
    unlock_cache(cache, idxMutex);
    return EOOM;
  }
  unlock_cache(cache, idxMutex);
  return OK;
}

enum code del(Cache cache, Stats stats, char *key)
{
  stats_ndel(stats);
  int idxMutex = lock_cache(cache, key);
  int i = delete_in_hashtable(cache->table, key);
  unlock_cache(cache, idxMutex);
  if (i)
  {
    delete_in_concurrent_queue(cache->queue, key);
    return OK;
  }
  return ENOTFOUND;
}

enum code get(Cache cache, Stats stats, int mode, char *key, char **val, unsigned int *vlen)
{
  stats_nget(stats);
  int idxMutex = lock_cache(cache, key);
  Data found = search_hashtable(cache->table, key);
  if (found == NULL)
  {
    unlock_cache(cache, idxMutex);
    return ENOTFOUND;
  }
  if (mode == TEXT_MODE && found->mode == BIN_MODE)
  {
    unlock_cache(cache, idxMutex);
    return EBINARY;
  }
  int flag_enomem = 0;
  ConcurrentQueue concq = cache->queue;
  update_queue(concq, key, &flag_enomem);
  if (flag_enomem)
  {
    unlock_cache(cache, idxMutex);
    return EOOM;
  }
  *vlen = found->vlen;
  if (try_malloc(sizeof(int) * (*vlen), (void *)val) == -1)
  {
    unlock_cache(cache, idxMutex);
    return EOOM;
  }
  memcpy(*val, found->val, *vlen);
  unlock_cache(cache, idxMutex);
  return OK;
}

Stats create_stats()
{
  Stats stats;
  if (try_malloc(sizeof(struct _Stats), (void *)&stats) == -1)
  {
    return NULL;
  }
  stats->nput = 0;
  stats->nget = 0;
  stats->ndel = 0;
  pthread_mutex_init(&stats->mutexSt, NULL);
  return stats;
}

enum code get_stats(Stats *stats, Stats allStats)
{
  for (int i = 0; i < numofthreads; i++)
  {
    pthread_mutex_lock(&stats[i]->mutexSt);
    allStats->nput += stats[i]->nput;
    allStats->nget += stats[i]->nget;
    allStats->ndel += stats[i]->ndel;
    pthread_mutex_unlock(&stats[i]->mutexSt);
  }
  return OK;
}

int print_stats(Cache cache, Stats stats, char **res)
{
  if (try_malloc(sizeof(char) * MAX_BUF_SIZE, (void *)res) == -1)
  {
    return -1;
  }
  // Formatear el mensaje en el buffer
  uint64_t numKeys = hashtable_nelems(cache->table);
  int len = snprintf(*res, MAX_BUF_SIZE, "PUTS=%ld DELS=%ld GETS=%ld KEYS=%ld",
                     stats->nput, stats->ndel, stats->nget, numKeys);
  return len;
}

void destroy_stats(Stats stats)
{
  pthread_mutex_destroy(&stats->mutexSt);
  free(stats);
  return;
}

void stats_nput(Stats stats)
{
  pthread_mutex_lock(&stats->mutexSt);
  stats->nput++;
  pthread_mutex_unlock(&stats->mutexSt);
  return;
}

void stats_nget(Stats stats)
{
  pthread_mutex_lock(&stats->mutexSt);
  stats->nget++;
  pthread_mutex_unlock(&stats->mutexSt);
  return;
}

void stats_ndel(Stats stats)
{
  pthread_mutex_lock(&stats->mutexSt);
  stats->ndel++;
  pthread_mutex_unlock(&stats->mutexSt);
  return;
}
