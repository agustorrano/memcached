#include "hashtable.h"

HashTable create_hashtable(unsigned capacity, HashFunction hash) {
  HashTable table = malloc(sizeof(struct _HashTable));
  if (table == NULL) {
    errno = ENOMEM;
		perror("Initializing Structs");
		exit(EXIT_FAILURE);
  }
  table->elems = malloc(sizeof(DNode*) * capacity);
  if (table->elems == NULL) {
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

int hashtable_nelems(HashTable table) { 
  return table->numElems; 
}

unsigned int hashtable_capacity(HashTable table) { 
  return table->capacity; 
}

unsigned idx_hashtable(HashTable table, char* key) {
  return table->hash(key) % table->capacity;
}

void destroy_hashtable(HashTable table) {
  for (unsigned idx = 0; idx < table->capacity; ++idx)
    destroy_list(table->elems[idx]);
  free(table->elems);
  free(table);
  return;
}

void insert_hashtable(HashTable table, Data data, int* flag_enomem) {
  unsigned idx = idx_hashtable(table, data->key);
  table->numElems++;
  table->elems[idx] = insert_beginning_list(table->elems[idx], data, flag_enomem);
  return;
}

DNode* delete_in_hashtable(HashTable table, char* key) {
  unsigned idx = idx_hashtable(table, key);
  DNode* node = delete_in_list(table->elems[idx], key);
  if (node)
    table->numElems--;
  return node;
}
