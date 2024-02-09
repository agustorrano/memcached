#ifndef __COMMAND_H__
#define __COMMAND_H__
#include "concqueue.h"
#include "common.h"
#include "hashtable.h"

#define TEXT_MODE 0
#define BIN_MODE 1
#define MAX_BUF_SIZE 2048
#define MAX_READ 2048*5
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


void lock_cache(Cache cache, int idxMutex);

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


//! @brief Representa el comando PUT de la memcached.
//!
//! @param[in] cache - Cache.
//! @param[in] stats - Stats.
//! @param[in] val - char* : valor que se quiere guardar.
//! @param[in] key - char* : clave del valor a guardar.
//! @param[in] mode - int : tipo de protocolo (texto o binario).
//! @param[in] vlen - int : longitud del valor que se quiere guardar.
//! @return command - enum code : OK
enum code put(Cache cache, Stats stats, char *val, char *key, int mode, int vlen);


//! @brief Representa el comando DEL de la memcached.
//!
//! @param[in] cache - Cache.
//! @param[in] stats - Stats.
//! @param[in] key - char* : clave del valor que se quiere eliminar.
//! @return command - enum code : OK en caso de que el dato existía y se eliminó,
//! ENOTFOUND en caso contrario.
enum code del(Cache cache, Stats stats, char *key);


//! @brief Representa el comando GET de la memcached.
//!
//! @param[in] cache - Cache.
//! @param[in] stats - Stats.
//! @param[in] mode - int : tipo de protocolo (texto o binario).
//! @param[in] key - char* : clave del valor que se quiere obtener.
//! @param[out] val - char** : dirección del buffer donde se almacenará el valor encontrado.
//! @param[out] vlen - int* : dirección del entero donde se guardará la longitud del valor encontrado.
enum code get(Cache cache, Stats stats, int mode, char *key, char** val, int* vlen);


//! @brief Crea una estructura tipo Stats.
//!
//! @return stats - Stats : estructura Stats creada (o NULL en caso de error).
Stats create_stats();


//! @brief Representa el comando STATS de la memcached.
//!
//! @param[in] stats - Stats* : array de las Stats de cada thread.
//! @param[out] allStats - Stats : estructura donde se guardará la suma total.
enum code get_stats(Stats* stats, Stats allStats);

//! @brief Accede al elemento numElems de la cache, de manera concurrente.
uint64_t get_numElems_concurrent(Cache cache);

//! @brief Formatea la respuesta al pedido stats
//!
//! @param[in] cache - Cache.
//! @param[in] stats - Stats.
//! @param[out] res - char** : dirección del buffer donde se almacenará la respuesta.
//! @return len - int : la longitud del mensaje, o -1 si ocurrió un error.
int print_stats(Cache cache, Stats stats, char** res);


//! @brief Destruye una estructura tipo Stats.
//!
//! @param[in] stats - Stats.
void destroy_stats(Stats stats);


//! @brief Incrementa en 1 el número de puts realizado.
//!
//! @param[out] cache - Cache.
void stats_nput(Stats stats);


//! @brief Incrementa en 1 el número de gets realizado.
//!
//! @param[out] cache - Cache.
void stats_nget(Stats stats);


//! @brief Incrementa en 1 el número de dels realizado.
//!
//! @param[out] cache - Cache.
void stats_ndel(Stats stats);

#endif