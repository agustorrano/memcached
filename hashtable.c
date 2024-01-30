#include "hashtable.h"

HashTable create_hashtable(unsigned capacity, HashFunction hash) {
  HashTable table;
  if (try_malloc(sizeof(struct _HashTable), (void*)&table) == -1) {
    errno = ENOMEM;
		perror("Initializing Structs");
		exit(EXIT_FAILURE);
  }
  if (try_malloc(sizeof(List) * capacity, (void*)&table->elems) == -1) {
    errno = ENOMEM;
		perror("Initializing Structs");
		exit(EXIT_FAILURE);
  }
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

void insert_hashtable(HashTable table, Data data, int* flag_enomem) {
  unsigned idx = table->hash(data->key) % table->capacity;
  Data found = search_list(table->elems[idx], data->key);
  /* si ya hay un valor asociado a key, es pisado */
  if (found != NULL) 
    strcpy(found->val, data->val);
  else {
    table->numElems++;
    /* int loadfactor = (table->numElems * 100) / table->capacity;
    if (loadfactor > 75) {
      rehash_hashtable(table);
      idx = table->hash(data->key) % table->capacity;
    } */
    table->elems[idx] = insert_beginning_list(table->elems[idx], data, flag_enomem);
  }
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

/*
void rehash_hashtable(HashTable table) {
  unsigned oldCap = table->capacity;
  table->capacity = table->capacity * 2;
  
  //alocamos memoria para el nuevo arreglo
  List* newArray;
  try_malloc(sizeof(List) * table->capacity, (void*)&newArray);
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
*/

int delete_in_hashtable(HashTable table, char* key) {
  unsigned idx = table->hash(key) % table->capacity;
  Data found = search_list(table->elems[idx], key);
  int i = 0;
  if (found != NULL) {
    table->numElems--;
    table->elems[idx] = delete_in_list(table->elems[idx], key);
    i = 1;
  }
  return i;
}
