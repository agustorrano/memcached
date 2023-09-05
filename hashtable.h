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

//! @struct _Stats
//! @brief Estructura que representa las estadísticas de acceso a la caché.
//! @var nput - unsigned int : número de puts realizados.
//! @var nget - unsigned int : número de gets realizados.
//! @var ndel - unsigned int : número de dels realizados.
struct _Stats {
  unsigned int nput, nget, ndel;
};

//! @typedef
typedef struct _Stats *Stats;

//! @struct _Cache
//! @brief Estructura que representa la caché.
//! @var table - HashTable.
//! @var mutexTh - pthread_mutex_t : mutex de la tabla hash.
//! @var stats - Stats.
//! @var mutexSt - pthread_mutex_t : mutex de la estructura Stats.
struct _Cache {
  HashTable table;
  pthread_mutex_t mutexTh;
  Stats stats;
  pthread_mutex_t mutexSt;
}; 

//! @typedef
typedef struct _Cache *Cache;

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


//! @brief Inicializa una estructura tipo caché.
//! 
//! Permite concurrencia.
//!
//! @param[in] cache - Cache.
//! @param[in] capacity - int : capacidad que tendrá la tabla dentro de la caché.
//! @param[in] hash - HashFunction : función hash para la tabla.
void init_cache(Cache cache, int capacity, HashFunction hash);


//! @brief Inserta un dato en la caché.
//! 
//! @param[in] cache - Cache.
//! @param[in] data - Data : dato a insertar.
void insert_cache(Cache cache, Data data);


//! @brief Destruye la caché.
//! 
//! @param[in] cache - Cache.
void destroy_cache(Cache cache);


//! @brief Verifica si el dato está en la caché.
//! 
//! @param[in] cache - Cache.
//! @param[in] key - char* : clave del dato a buscar.
//! @return data - Data : dato encontrado (o NULL).
Data search_cache(Cache cache, char* key);


//! @brief Elimina un dato de la caché.
//! 
//! @param[in] cache - Cache.
//! @param[in] key - char* : clave del dato a buscar.
//! @param[out] flag - int* : bandera que determina si efectivamente
//! se eliminó el dato.
void delete_in_cache(Cache cache, char* key, int *flag);

//! @brief Crea una estructura tipo Stats.
//!
//! @return stats - Stats : estructura Stats creada.
Stats create_stats();


//! @brief Destruye una estructura tipo Stats.
//!
//! @param[in] stats - Stats.
void destroy_stats(Stats stats);


//! @brief Incrementa en 1 el número de puts realizado.
//!
//! @param[out] cache - Cache.
void stats_nput(Cache cache);


//! @brief Incrementa en 1 el número de gets realizado.
//!
//! @param[out] cache - Cache.
void stats_nget(Cache cache);


//! @brief Incrementa en 1 el número de dels realizado.
//!
//! @param[out] cache - Cache.
void stats_ndel(Cache cache);

#endif /* __TABLAHASH_H__ */