#ifndef __COMMAND_H__
#define __COMMAND_H__
#include "concqueue.h"
#include "common.h"
#include "hashtable.h"

#define TEXT_MODE 0
#define BIN_MODE 1
#define MAX_BUF_SIZE 2048
#define MAX_READ 2048 * 5
#define NUM_MUTEX 100
#define NUM_BLOCKS 10000

//! @struct _Stats
//! @brief Estructura que representa las estadísticas de acceso a la caché.
//! @var nput - uint64_t : número de puts realizados.
//! @var nget - uint64_t : número de gets realizados.
//! @var ndel - uint64_t : número de dels realizados.
//! @var mutexSt - pthread_mutex_t : mutex de la estructura Stats.
struct _Stats
{
  uint64_t nput, nget, ndel;
  pthread_mutex_t mutexSt;
};

//! @typedef
typedef struct _Stats *Stats;

//! @struct _Cache
//! @brief Estructura que representa la caché.
//! @var table - HashTable.
//! @var mutexTh - pthread_mutex_t[NUM_MUTEX] : array de
//! mutex de las distintas secciones de la tabla hash.
//! @var queue - ConcurrentQueue.
struct _Cache
{
  HashTable table;
  pthread_mutex_t mutexTh[NUM_MUTEX];
  ConcurrentQueue queue;
};

//! @typedef
typedef struct _Cache *Cache;

extern Cache cache;
extern Stats *statsTh;

//! @brief Calcula, segun la clave dada, que partición de la caché
//! debe restringir. Realiza un mutex_lock para la misma.
//!
//! @param[in] cache - Cache.
//! @param[in] key - char*: clave a la cual se quiere acceder.
int lock_cache(Cache cache, char *key);

//! @brief Realiza un mutex_unlock para la partición de la caché
//! según el índice pasado como argumento.
//!
//! @param[in] cache - Cache.
//! @param[in] idxMUtex - int: indice de la sección que se realizará el unlock.
void unlock_cache(Cache cache, int idxMutex);

//! @brief Calcula que partición de la caché debe restringir.
//!
//! @param[in] idx - int: idx del casillero de la tabla hash.
int idx_mutex(unsigned idx);

//! @brief Inicializa una estructura tipo caché.
//!
//! Permite concurrencia.
//!
//! @param[in] cache - Cache.
//! @param[in] queue - ConcurrentQueue.
//! @param[in] capacity - int : capacidad que tendrá la tabla dentro de la caché.
//! @param[in] hash - HashFunction : función hash para la tabla.
void init_cache(Cache cache, ConcurrentQueue queue, unsigned capacity, HashFunction hash);

//! @brief Destruye la caché.
//!
//! @param[in] cache - Cache.
void destroy_cache(Cache cache);

//! @brief Elimina un dato de la caché.
//!
//! @param[in] cache - Cache.
//! @param[in] key - char* : clave del dato a buscar.
//! @return data - int : 1 si fue eliminado (0 si no).
int delete_in_cache(Cache cache, char *key, int idxMutex);

//! @brief Representa el comando PUT de la memcached.
//!
//! @param[in] cache - Cache.
//! @param[in] stats - Stats.
//! @param[in] val - char* : valor que se quiere guardar.
//! @param[in] key - char* : clave del valor a guardar.
//! @param[in] mode - int : tipo de protocolo (texto o binario).
//! @param[in] vlen - int : longitud del valor que se quiere guardar.
//! @return command - enum code : OK - EOOM.
enum code put(Cache cache, Stats stats, char *val, char *key, int mode, unsigned int vlen);

//! @brief Representa el comando DEL de la memcached.
//!
//! @param[in] cache - Cache.
//! @param[in] stats - Stats.
//! @param[in] key - char* : clave del valor que se quiere eliminar.
//! @return command - enum code : OK - ENOTFOUND.
enum code del(Cache cache, Stats stats, char *key);

//! @brief Representa el comando GET de la memcached.
//!
//! @param[in] cache - Cache.
//! @param[in] stats - Stats.
//! @param[in] mode - int : tipo de protocolo (texto o binario).
//! @param[in] key - char* : clave del valor que se quiere obtener.
//! @param[out] val - char** : dirección del buffer donde se almacenará el valor encontrado.
//! @param[out] vlen - unsigned int* : dirección del entero donde se guardará la longitud del valor encontrado.
//! @return command - enum code : OK  - ENOTFOUND - EBINARY - EOOM.
enum code get(Cache cache, Stats stats, int mode, char *key, char **val, unsigned int *vlen);

//! @brief Crea una estructura tipo Stats.
//!
//! @return stats - Stats : estructura Stats creada (o NULL en caso de error).
Stats create_stats();

//! @brief Representa el comando STATS de la memcached.
//!
//! @param[in] stats - Stats* : array de las Stats de cada thread.
//! @param[out] allStats - Stats : estructura donde se guardará la suma total.
//! @return command - enum code : OK.
enum code get_stats(Stats *stats, Stats allStats);

//! @brief Formatea la respuesta al pedido stats
//!
//! @param[in] cache - Cache.
//! @param[in] stats - Stats.
//! @param[out] res - char** : dirección del buffer donde se almacenará la respuesta.
//! @return len - unsigned int : la longitud del mensaje, o 0 si ocurrió un error.
unsigned int print_stats(Cache cache, Stats stats, char **res);

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