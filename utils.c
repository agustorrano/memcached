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
	int intentos;
	int MAX_INTENTOS = 15;
	for (intentos = 0; intentos < MAX_INTENTOS && *ptr == NULL; intentos++){ // habria que poner algun limite.
		release_memory(cache);
		*ptr = malloc(size);
	}
	return intentos;
}

unsigned KRHash(char *s) {
  unsigned hashval;
  for (hashval = 0; *s != '\0'; ++s) 
    hashval = *s + 31 * hashval;
  return hashval;
}

Data create_data(char* val, char* key, int mode, int vlen) {
  //Data data = malloc(sizeof(struct _Data));
  Data data;
  try_malloc(sizeof(struct _Data), (void*)&data);
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
  //char *val = malloc(sizeof(char) * (1 + strlen(data->val)));
  //char *key = malloc(sizeof(char) * (1 + strlen(data->key)));
  char* val; char* key;
  try_malloc(sizeof(char) * (1 + strlen(data->val)), (void*)&val);
  try_malloc(sizeof(char) * (1 + strlen(data->key)), (void*)&key);
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
  printf(" Valor: %s, Clave: %s\t", data->val, data->key);
  if (data->mode == 0)
    printf(", Modo: binario\n");
  else 
    printf(", Modo: texto\n");
  return;
}
