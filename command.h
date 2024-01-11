#ifndef __COMMAND_H__
#define __COMMAND_H__
#include "concqueue.h"
#include "hashtable.h"
#include "utils.h"

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

Cache cache;

void put(Cache cache, ConcurrentQueue queue, char *val, char *key);

void del(Cache cache, ConcurrentQueue queue, char *key);

char *get(Cache cache, ConcurrentQueue queue, char *key);

void get_stats(Cache cache);


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

#endif