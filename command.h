#ifndef __COMMAND_H__
#define __COMMAND_H__
#include "concqueue.h"
#include "common.h"
#include "hashtable.h"

#define TEXT_MODE 0
#define BIN_MODE 1

//! @struct _Stats
//! @brief Estructura que representa las estadísticas de acceso a la caché.
//! @var nput - unsigned int : número de puts realizados.
//! @var nget - unsigned int : número de gets realizados.
//! @var ndel - unsigned int : número de dels realizados.
struct _Stats {
  unsigned int nput, nget, ndel;
  pthread_mutex_t mutexSt;
};

//! @typedef
typedef struct _Stats *Stats;

typedef struct eventloop_data {
	int epfd; // file descriptor para epoll
	int text_sock;
	int bin_sock;
	int id;
  int nproc;
} eventloopData;

//! @struct _Cache
//! @brief Estructura que representa la caché.
//! @var table - HashTable.
//! @var mutexTh - pthread_mutex_t : mutex de la tabla hash.
//! @var textSt - Stats.
//! @var binSt - Stats.
//! @var mutexTextSt - pthread_mutex_t : mutex de la estructura Stats con protocolo de texto.
//! @var mutexBinSt - pthread_mutex_t : mutex de la estructura Stats con protocolo binario.
struct _Cache {
  HashTable table;
  pthread_mutex_t mutexTh;
  Stats stats;
}; 

//! @typedef
typedef struct _Cache *Cache;

//! @brief Representa ell comando PUT de la memcached.
//!
//! @param[in] cache - Cache.
//! @param[in] queue - ConcurrentQueue.
//! @param[in] val - char* : valor que se quiere guardar.
//! @param[in] key - char* : clave del valor a guardar.
//! @param[in] mode - int : tipo de protocolo (texto o binario).
void put(Cache cache, ConcurrentQueue queue, Stats stats, char *val, char *key, int mode);

//! @brief Representa ell comando DEL de la memcached.
//!
//! @param[in] cache - Cache.
//! @param[in] queue - ConcurrentQueue.
//! @param[in] key - char* : clave del valor que se quiere eliminar.
//! @param[in] mode - int : tipo de protocolo (texto o binario).
int del(Cache cache, ConcurrentQueue queue, Stats stats, char *key);

//! @brief Representa ell comando GET de la memcached.
//!
//! @param[in] cache - Cache.
//! @param[in] queue - ConcurrentQueue.
//! @param[in] key - char* : clave del valor que se quiere obtener.
//! @param[in] mode - int : tipo de protocolo (texto o binario).
char *get(Cache cache, ConcurrentQueue queue, Stats stats, char *key);

//! @brief Representa ell comando STATS de la memcached.
//!
//! @param[in] cache - Cache.
//! @param[in] fd - int : fd del cliente que pidió la información.
//! @param[in] mode - int : tipo de protocolo (texto o binario).
void get_stats(Cache cache, Stats* stats, int fd, int nproc);


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
int delete_in_cache(Cache cache, char* key);

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