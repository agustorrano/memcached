#include "lru.h"
#include "command.h"

void release_memory(Cache cache){
  uint64_t numData = hashtable_nelems(cache->table);
	int numDelete;
  if (0 < numData < 10) numDelete = 1;
  else numDelete = 0.1 * numData;
	char* delKey;
  pthread_mutex_lock(&cache->queue->mutex);
  DNode* node = cache->queue->queue->first;
	for (int i = 0; i < numDelete, node != NULL; node = node->next) {
    unsigned idx = idx_hashtable(cache->table, node->key);
    int idxMutex = idx_mutex(idx);
    if (pthread_mutex_trylock(&cache->mutexTh[idxMutex]) != 0)
      continue;
    delete_in_hashtable(cache->table, node->key);
		remove_from_queue(cache->queue->queue, node);
    i++;
    pthread_mutex_unlock(&cache->mutexTh[idxMutex]);
	}
  pthread_mutex_unlock(&cache->queue->mutex);
}

int try_malloc(size_t size, void** ptr){
	int MAX_ATTEMPTS = 10;
  *ptr = malloc(size);
	for (int at = 0; at < MAX_ATTEMPTS && *ptr == NULL; at++){
		release_memory(cache);
		*ptr = malloc(size);
	}
	if (*ptr == NULL) { 
    // perror("cannot allocate in try malloc: ");
    return -1;
  }
  else { return 0; }
}