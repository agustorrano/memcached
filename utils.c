#include "command.h"
#include "utils.h"

long numofthreads;

/* mutex recursivo */
void config_mutex(pthread_mutex_t* mtx) {
  pthread_mutexattr_t mtxAttr;
  int s = pthread_mutexattr_init(&mtxAttr);
  if (s != 0) {
		perror("pthread_mutexattr_init");
		exit(EXIT_FAILURE);
  }
  s = pthread_mutexattr_settype(&mtxAttr, PTHREAD_MUTEX_RECURSIVE);
  if (s != 0) {
    perror("pthread_mutexattr_settype");
		exit(EXIT_FAILURE);
  }
  s = pthread_mutex_init(mtx, &mtxAttr); 
  if (s != 0) {
    perror("pthread_mutex_init");
		exit(EXIT_FAILURE);
  }
  s = pthread_mutexattr_destroy(&mtxAttr);
  if (s != 0) {
    perror("pthread_mutexattr_destroy");
		exit(EXIT_FAILURE);
  }
}

void release_memory(Cache cache){
  pthread_mutex_lock(&cache->mutexTh);
  uint64_t numData = get_numElems_concurrent(cache);
  // uint64_t numData = cache->table->numElems;
	int numDelete = 0.1 * numData; // liberamos el 10%?
	char* delKey;
	for (int i = 0; i < numDelete; i++) {
    log(1, "I Want: Queue Mutex!");
		//delKey = pop_concurrent_queue(cache->queue);
    if (delKey != NULL) {
      log(1, "I Want: Cache Mutex!");
      int res = 0;
		  //int res = delete_in_cache(cache, delKey);
      if (res == 1) {log(1, "Memory Released!");}
      if (res == 0) {
        log(1, "Not Released!");
        break;
      }
    }
	}
  pthread_mutex_unlock(&cache->mutexTh);
}

int try_malloc(size_t size, void** ptr){
	int MAX_ATTEMPTS = 10;
  *ptr = malloc(size);
	for (int at = 0; at < MAX_ATTEMPTS && *ptr == NULL; at++){
    log(1, "Trying to release memory");
		release_memory(cache);
		*ptr = malloc(size);
	}
  //if (rand() % 10 == 0) {
  //  log(1, "Trying to release memory");
	//  release_memory(cache);
  //}
	// *ptr = malloc(size);
	if (*ptr == NULL) { 
    perror("cannot allocate in try malloc: ");
    return -1;
  }
  else { 
    return 0; }
}

unsigned KRHash(char *s) {
  unsigned hashval;
  for (hashval = 0; *s != '\0'; ++s) 
    hashval = *s + 31 * hashval;
  return hashval;
}

Data create_data(char* val, char* key, int mode, int vlen) {
  Data data;
  if (try_malloc(sizeof(struct _Data), (void*)&data) == -1) { return NULL; }
  data->mode = mode;
  data->key = key;
  data->val = val;
  data->vlen = vlen;
  return data;
}

void destroy_data(Data data) { 
  free(data->key);
  free(data->val);
  free(data); 
  return;
}

Data copy_data(Data data) {
  char* val; char* key;
  size_t size1 = sizeof(char) * (1 + strlen(data->val));
  if (try_malloc(size1, (void*)&val) == -1) {
    return NULL;
  }
  size1 = sizeof(char) * (1 + strlen(data->key));
  if (try_malloc(size1, (void*)&key) == -1) {
    return NULL;
  }
  strcpy(val, data->val);
  strcpy(key, data->key);
  int mode = data->mode;
  int vlen = data->vlen;
  Data newData = create_data(val, key, mode, vlen);
  return newData;
}

int compare_data(char* key1, char* key2) {
  return !strcmp(key1, key2);
}

void print_data(Data data) {
  printf(" Valor: %s, Clave: %s, Longitud del valor: %d\t", data->val, data->key, data->vlen);
  if (data->mode == 0)
    printf(", Modo: texto\n");
  else 
    printf(", Modo: binario\n");
  return;
}
