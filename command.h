#ifndef __COMMAND_H__
#define __COMMAND_H__

#include "cache.h"
#include "common.h"

#define TEXT_MODE 0
#define BIN_MODE 1
#define MAX_BUF_SIZE 2048
#define MAX_READ 2048*5


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



//! @brief Representa el comando STATS de la memcached.
//!
//! @param[in] stats - Stats* : array de las Stats de cada thread.
//! @param[out] allStats - Stats : estructura donde se guardará la suma total.
enum code get_stats(Stats* stats, Stats allStats);


//! @brief Formatea la respuesta al pedido stats
//!
//! @param[in] cache - Cache.
//! @param[in] stats - Stats.
//! @param[out] res - char** : dirección del buffer donde se almacenará la respuesta.
//! @return len - int : la longitud del mensaje, o -1 si ocurrió un error.
int print_stats(Cache cache, Stats stats, char** res);


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