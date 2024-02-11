#include "hashtable.h"
#include "log.h"

unsigned idx_hashtable(HashTable table, char* key) {
  return table->hash(key) % table->capacity;
}

HashTable create_hashtable(unsigned capacity, HashFunction hash) {
  HashTable table = malloc(sizeof(struct _HashTable));
  if (table == NULL) {
    errno = ENOMEM;
		perror("Initializing Structs");
		exit(EXIT_FAILURE);
  }
  table->elems = malloc(sizeof(List) * capacity);
  if (table->elems == NULL) {
    errno = ENOMEM;
		perror("Initializing Structs");
		exit(EXIT_FAILURE);
  }
  table->numElems = 0;
  pthread_mutex_init(&table->mutexNumE, NULL);
  table->capacity = capacity;
  table->hash = hash;
  for (unsigned idx = 0; idx < capacity; ++idx)
    table->elems[idx] = create_list();
  return table;
}

uint64_t hashtable_nelems(HashTable table) { 
  uint64_t numElems;
  pthread_mutex_lock(&table->mutexNumE);
  numElems = table->numElems;
  pthread_mutex_unlock(&table->mutexNumE);
  return numElems; 
}

void destroy_hashtable(HashTable table) {
  for (unsigned idx = 0; idx < table->capacity; ++idx)
    destroy_list(table->elems[idx]);
  free(table->elems);
  pthread_mutex_destroy(&table->mutexNumE);
  free(table);
  return;
}

void insert_hashtable(HashTable table, Data data, int* flag_enomem) {
  int idx = idx_hashtable(table, data->key);
  Data found = search_list(table->elems[idx], data->key);
  /* si ya hay un valor asociado a key, es pisado */
  if (found != NULL) {
    strcpy(found->val, data->val);
    found->vlen = data->vlen;
    found->mode = data->mode;
  }
  else {
    pthread_mutex_lock(&table->mutexNumE);
    table->numElems++;
    pthread_mutex_unlock(&table->mutexNumE);
    table->elems[idx] = insert_beginning_list(table->elems[idx], data, flag_enomem);
  }
  return;
} 

Data search_hashtable(HashTable table, char* key) {
  unsigned idx = table->hash(key) % table->capacity;
  return search_list(table->elems[idx], key);
}

int delete_in_hashtable(HashTable table, char* key) {
  unsigned idx = idx_hashtable(table, key);
  Data found = search_list(table->elems[idx], key);
  int i = 0;
  if (found != NULL) {
    pthread_mutex_lock(&table->mutexNumE);
    table->numElems--;
    pthread_mutex_unlock(&table->mutexNumE);
    table->elems[idx] = delete_in_list(table->elems[idx], key);
    i = 1;
  }
  return i;
}
