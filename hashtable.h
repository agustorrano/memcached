#ifndef __TABLAHASH_H__
#define __TABLAHASH_H__
#include "list.h"
#include "utils.h"

//! @struct _HashTable
//! @brief Estructura que representa la tabla hash.
//! @var elems - List * : arreglo de listas enlazadas.
//! @var numElems - unsigned : cantidad de elementos en la tabla.
//! @var capacity - unsigned : capacidad de la tabla.
//! @var hash - HashFunction.
struct _HashTable {
  List *elems;
  unsigned numElems;
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
//! @param[in] table - HashTable.
//! @return numElems - int: cantidad de elementos en la tabla.
int hashtable_nelems(HashTable table);


//! @brief Retorna la capacidad de la tabla.
//!
//! @param[in] table - HashTable.
//! @return capacity - unsigned: capacidad actual de la tabla.
unsigned hashtable_capacity(HashTable table);


//! @brief Destruye la tabla.
//!
//! @param[in] table - HashTable.
void destroy_hashtable(HashTable table);


//! @brief Inserta un dato en la tabla
//!
//! Si el dato ya se encontraba, no hace nada. 
//! Dependiendo del factor de carga aumenta la capacidad de la tabla.
//!
//! @param[in] table - HashTable.
//! @param[in] data - Data: dato a insertar.
void insert_hashtable(HashTable table, Data data);


//! @brief Verifica si el dato está en la tabla.
//!
//! @param[in] table - HashTable.
//! @param[in] key - char* : clave del dato a buscar.
//! @return data - Data : dato encontrado (o NULL).
Data search_hashtable(HashTable table, char* key);


//! @brief Realiza un map en la tabla.
//!
//! @param[in] table - HashTable.
//! @param[in] visit - VisitFunction : función aplicada a cada elemento.
void map_hashtable(HashTable table, VisitFunction visit);


//! @brief Realiza un rehash.
//! 
//! Se llama a esta funcion cuando el factor de carga de la tabla 
//! anterior era mayor al recomendado.
//! Duplica la capacidad de la tabla.
//!
//! @param[in] table - HashTable.
void rehash_hashtable(HashTable table);


//! @brief Elimina un dato de la tabla.
//! 
//! Si el dato no está presente, no hace nada.
//!
//! @param[in] table - HashTable.
//! @param[in] key - char* : clave del dato a buscar.
//! @param[out] flag - int* : bandera que determina si efectivamente
//! se eliminó el dato.
void delete_in_hashtable(HashTable table, char* key, int* flag);

#endif /* __TABLAHASH_H__ */