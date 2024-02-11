#ifndef __CACHE_H__
#define __CACHE_H__

#include "concqueue.h"
#include "hashtable.h"

#define NUM_MUTEX 100
#define NUM_BLOCKS 10000

//! @struct _Stats
//! @brief Estructura que representa las estadísticas de acceso a la caché.
//! @var nput - unsigned int : número de puts realizados.
//! @var nget - unsigned int : número de gets realizados.
//! @var ndel - unsigned int : número de dels realizados.
//! @var mutexSt - pthread_mutex_t : mutex de la estructura Stats.
struct _Stats {
  uint64_t nput, nget, ndel;
  pthread_mutex_t mutexSt;
};

//! @typedef
typedef struct _Stats *Stats;

//! @struct _Cache
//! @brief Estructura que representa la caché.
//! @var table - HashTable.
//! @var mutexTh - pthread_mutex_t : mutex de la tabla hash.
//! @var queue - ConcurrentQueue : mutex de la tabla hash.
struct _Cache {
  HashTable table;
  pthread_mutex_t mutexTh[NUM_MUTEX];
  ConcurrentQueue queue;
}; 

//! @typedef
typedef struct _Cache *Cache;

extern Cache cache;
extern Stats* statsTh;


int lock_cache(Cache cache, char* key);

void unlock_cache(Cache cache, int idxMutex);

int idx_mutex(unsigned idx);

//! @brief Inicializa una estructura tipo caché.
//! 
//! Permite concurrencia.
//!
//! @param[in] cache - Cache.
//! @param[in] queue - ConcurrentQueue.
//! @param[in] capacity - int : capacidad que tendrá la tabla dentro de la caché.
//! @param[in] hash - HashFunction : función hash para la tabla.
void init_cache(Cache cache, ConcurrentQueue queue, int capacity, HashFunction hash);


//! @brief Inserta un dato en la caché.
//! 
//! @param[in] cache - Cache.
//! @param[in] data - Data : dato a insertar.
//! @param[out] flag_enomem - int* : bandera para informar que no se pudo allocar memoria.
void insert_cache(Cache cache, Data data, int* flag_enomem);


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
//! @return data - int : 1 si fue eliminado (0 si no).
int delete_in_cache(Cache cache, char* key, int idxMutex);

//! @brief Crea una estructura tipo Stats.
//!
//! @return stats - Stats : estructura Stats creada (o NULL en caso de error).
Stats create_stats();

//! @brief Destruye una estructura tipo Stats.
//!
//! @param[in] stats - Stats.
void destroy_stats(Stats stats);

#endif