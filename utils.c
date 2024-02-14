#define _GNU_SOURCE
#include "command.h"
#include "utils.h"

long numofthreads;

void config_mutex(pthread_mutex_t *mtx)
{
  int s = pthread_mutex_init(mtx, NULL);
  if (s != 0)
  {
    perror("pthread_mutex_init");
    exit(EXIT_FAILURE);
  }
}

void config_recursive_mutex(pthread_mutex_t *mtx)
{
  pthread_mutexattr_t mtxAttr;
  int s = pthread_mutexattr_init(&mtxAttr);
  if (s != 0)
  {
    perror("pthread_mutexattr_init");
    exit(EXIT_FAILURE);
  }
  s = pthread_mutexattr_settype(&mtxAttr, PTHREAD_MUTEX_RECURSIVE);
  if (s != 0)
  {
    perror("pthread_mutexattr_settype");
    exit(EXIT_FAILURE);
  }
  s = pthread_mutex_init(mtx, &mtxAttr);
  if (s != 0)
  {
    perror("pthread_mutex_init");
    exit(EXIT_FAILURE);
  }
  s = pthread_mutexattr_destroy(&mtxAttr);
  if (s != 0)
  {
    perror("pthread_mutexattr_destroy");
    exit(EXIT_FAILURE);
  }
}

// Función principal del desalojo de memoria.
// Intenta liberar el 10% de la cache, para ello, utiliza la
// Cola de prioridades. 
void release_memory(Cache cache)
{
  uint64_t numData = hashtable_nelems(cache->table);
  int numDelete;
  if (0 < numData && numData < 10)
    numDelete = 1;
  else
    numDelete = 0.1 * numData;
  pthread_mutex_lock(&cache->queue->mutex);
  DNode *node = cache->queue->queue->first;
  for (int i = 0; i < numDelete && node != NULL;)
  {
    unsigned idx = idx_hashtable(cache->table, node->key);
    int idxMutex = idx_mutex(idx);
    if (pthread_mutex_trylock(&cache->mutexTh[idxMutex]) != 0)
    {
      node = node->next;
      continue;
    }
    delete_in_hashtable(cache->table, node->key);
    node = remove_from_queue(cache->queue->queue, node, 1);
    i++;
    pthread_mutex_unlock(&cache->mutexTh[idxMutex]);
  }
  pthread_mutex_unlock(&cache->queue->mutex);
}

int try_malloc(size_t size, void **ptr)
{
  int MAX_ATTEMPTS = 10;
  *ptr = malloc(size);
  for (int at = 0; at < MAX_ATTEMPTS && *ptr == NULL; at++)
  {
    release_memory(cache);
    *ptr = malloc(size);
    log(1, "Memory Released");
  }
  if (*ptr == NULL)
  {
    return -1;
  }
  else
  {
    return 0;
  }
}

unsigned KRHash(char *s)
{
  unsigned hashval;
  for (hashval = 0; *s != '\0'; ++s)
    hashval = *s + 31 * hashval;
  return hashval;
}

Data create_data(char *val, char *key, int mode, unsigned int vlen)
{
  Data data;
  if (try_malloc(sizeof(struct _Data), (void *)&data) == -1)
  {
    return NULL;
  }
  data->mode = mode;
  data->key = key;
  data->val = val;
  data->vlen = vlen;
  return data;
}

void destroy_data(Data data)
{
  free(data);
  return;
}

Data copy_data(Data data)
{
  char *val;
  char *key;

  if (try_malloc((sizeof(char) * (data->vlen + 1)), (void *)&val) == -1)
  {
    return NULL;
  }
  size_t size = sizeof(char) * (1 + strlen(data->key));
  if (try_malloc(size, (void *)&key) == -1)
  {
    return NULL;
  }
  memcpy(val, data->val, (data->vlen + 1));
  memcpy(key, data->key, size);
  int mode = data->mode;
  unsigned int vlen = data->vlen;
  Data newData = create_data(val, key, mode, vlen);
  return newData;
}

int compare_data(char *key1, char *key2)
{
  return !strcmp(key1, key2);
}