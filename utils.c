#include "lru.h"
#include "utils.h"

long numofthreads;

/* mutex recursivo */
void config_mutex(pthread_mutex_t* mtx) {
  int s = pthread_mutex_init(mtx, NULL); 
  if (s != 0) {
    perror("pthread_mutex_init");
		exit(EXIT_FAILURE);
  }
}

void config_recursive_mutex(pthread_mutex_t* mtx) {
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

