#ifndef __TABLAHASH_H__
#define __TABLAHASH_H__
#include "list.h"

#define TABLE_INIT_CAPACITY 1000000

//! @struct _HashTable
//! @brief Estructura que representa la tabla hash.
//! @var elems - List * : arreglo de listas enlazadas.
//! @var numElems - uint64_t : cantidad de elementos en la tabla.
//! @var capacity - unsigned : capacidad de la tabla.
//! @var hash - HashFunction.
struct _HashTable
{
  List *elems;
  uint64_t numElems;
  pthread_mutex_t mutexNumE;
  unsigned capacity;
  HashFunction hash;
};

//! @typedef
typedef struct _HashTable *HashTable;

//! @brief Crea una tabla hash.
//!
//! @param[in] capacity - unsigned: capacidad que tendrá la tabla.
//! @param[in] hash - HashFunction: función hash para la tabla.
//! @return table - HashTable: tabla creada.
HashTable create_hashtable(unsigned capacity, HashFunction hash);

//! @brief Retorna el numero de elementos de la tabla.
//!
//! Permite concurrencia
//!
//! @param[in] table - HashTable.
//! @return numElems - uint64_t: cantidad de elementos en la tabla.
uint64_t hashtable_nelems(HashTable table);

//! @brief Destruye la tabla.
//!
//! @param[in] table - HashTable.
void destroy_hashtable(HashTable table);

//! @brief Inserta un dato en la tabla
//!
//! Si el dato ya se encontraba, no hace nada.
//!
//! @param[in] table - HashTable.
//! @param[in] data - Data: dato a insertar.
//! @param[out] flag_enomem - int* : bandera para informar que no se pudo allocar memoria.
void insert_hashtable(HashTable table, Data data, int *flag_enomem);

//! @brief Verifica si el dato está en la tabla.
//!
//! @param[in] table - HashTable.
//! @param[in] key - char* : clave del dato a buscar.
//! @return data - Data : dato encontrado (o NULL).
Data search_hashtable(HashTable table, char *key);

//! @brief Elimina un dato de la tabla.
//!
//! Si el dato no está presente, no hace nada.
//!
//! @param[in] table - HashTable.
//! @param[in] key - char* : clave del dato a buscar.
//! @return i - int : 1 si se eliminó el dato, 0 en caso contrario.
int delete_in_hashtable(HashTable table, char *key);

//! @brief Calcula el índice según la función hash y el dato a insertar
//!
//! @param[in] table - HashTable.
//! @param[in] key - char* : clave del dato a insertar
//! @return idx - int: índice calculado.
unsigned idx_hashtable(HashTable table, char *key);

#endif /* __TABLAHASH_H__ */