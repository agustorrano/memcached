#include "hashtable.h"

HashTable create_hashtable(unsigned capacity, HashFunction hash) {

  HashTable table = malloc(sizeof(struct _HashTable));
  assert(table != NULL);
  table->elems = malloc(sizeof(List) * capacity);
  assert(table->elems != NULL);
  table->numElems = 0;
  table->capacity = capacity;
  table->hash = hash;
  for (unsigned idx = 0; idx < capacity; ++idx)
    table->elems[idx] = create_list();
  return table;
}

int hashtable_nelems(HashTable table) { return table->numElems; }

unsigned int hashtable_capacity(HashTable table) { return table->capacity; }

void destroy_hashtable(HashTable table) {
  for (unsigned idx = 0; idx < table->capacity; ++idx)
    destroy_list(table->elems[idx]);
  free(table->elems);
  free(table);
  return;
}

void insert_hashtable(HashTable table, Data data) {
  unsigned idx = table->hash(data->key) % table->capacity;
  Data found = search_list(table->elems[idx], data->key);
  if (found != NULL) { 
    strcpy(found->val, data->val);
    return;
  }
  table->numElems++;
  int loadfactor = (table->numElems * 100) / table->capacity;
  if (loadfactor > 75) {
    rehash_hashtable(table);
    idx = table->hash(data->key) % table->capacity;
  }
  table->elems[idx] = insert_beginning_list(table->elems[idx], data);
  return;
} 

Data search_hashtable(HashTable table, char* key) {
  unsigned idx = table->hash(key) % table->capacity;
  return search_list(table->elems[idx], key);
}

void map_hashtable(HashTable table, VisitFunction visit) {
  for (unsigned i = 0; i < table->capacity; i++) {
    printf("%d", i); //indice del arreglo de casilleros
    map_list(table->elems[i], visit);
    printf("\n");
  }
  printf("\n");
  return;
}

void rehash_hashtable(HashTable table) {
  unsigned oldCap = table->capacity;
  table->capacity = table->capacity * 2;
  
  //alocamos memoria para el nuevo arreglo
  List *newArray = malloc(sizeof(List) * table->capacity);
  assert(newArray != NULL);

  // Inicializamos las casillas con datos nulos.
  for (unsigned idx = 0; idx < table->capacity; ++idx) 
    newArray[idx] = create_list();
  
  int key;
  for (unsigned idx = 0; idx < oldCap; ++idx) {
    for (Node *node = table->elems[idx]; node != NULL; node = node->next) {
      key = table->hash(node->data->key) % table->capacity;
      newArray[key] = insert_beginning_list(newArray[key], node->data);
    }
  }

  //destruimos el viejo arreglo  
  for (unsigned idx = 0; idx < oldCap; ++idx)
    if (!empty_list(table->elems[idx]))
      destroy_list(table->elems[idx]);
  free(table->elems);

  //agregamos el nuevo arreglo a la table
  table->elems = newArray;
  return;
}

void delete_in_hashtable(HashTable table, char* key, int* flag) {
  unsigned idx = table->hash(key) % table->capacity;
  Data found = search_list(table->elems[idx], key);
  if (found != NULL) {
    table->numElems--;
    table->elems[idx] = delete_in_list(table->elems[idx], key);
    *flag++;
  }
  return;
}

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

void delete_in_cache(Cache cache, char* key, int *flag) {
  pthread_mutex_lock(&cache->mutexTh);
  delete_in_hashtable(cache->table, key, flag);
  pthread_mutex_unlock(&cache->mutexTh);
  return;
}

Stats create_stats() {
  Stats stats = malloc(sizeof(struct _Stats));
  stats->nput = 0;
  stats->nget = 0;
  stats->ndel = 0;
  return stats;
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
