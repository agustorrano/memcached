#include "command.h"
#include "utils.h"

long numofthreads;

void release_memory(Cache cache){
	int numData = cache->table->numElems;
	int numDelete = 0.1 * numData; // liberamos el 10%?
	char* delKey;
	for (int i = 0; i < numDelete; i++) {
		delKey = pop_concurrent_queue(cache->queue);
		delete_in_cache(cache, delKey);
	}
}

int try_malloc(size_t size, void** ptr){
	*ptr = malloc(size);
	int MAX_ATTEMPTS = 10;
	for (int at = 0; at < MAX_ATTEMPTS && *ptr == NULL; at++){
		release_memory(cache);
		*ptr = malloc(size);
	}
	if (*ptr == NULL) { return -1; }
  else { return 0; }
}

unsigned KRHash(char *s) {
  unsigned hashval;
  for (hashval = 0; *s != '\0'; ++s) 
    hashval = *s + 31 * hashval;
  return hashval;
}

Data create_data(char* val, char* key, int mode, int vlen, int* flag_enomem) {
  Data data;
  if (try_malloc(sizeof(struct _Data), (void*)&data) == -1) {
    *flag_enomem = 1;
    return NULL;
  }
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

Data copy_data(Data data, int* flag_enomem) {
  char* val; char* key;
  if (try_malloc(sizeof(char) * (1 + strlen(data->val)), (void*)&val) == -1) {
    *flag_enomem = 1;
    return NULL;
  }
  if (try_malloc(sizeof(char) * (1 + strlen(data->key)), (void*)&key) == -1) {
    *flag_enomem = 1;
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
