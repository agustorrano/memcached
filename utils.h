#ifndef __UTILS_H__
#define __UTILS_H__

// #define _GNU_SOURCE

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <pthread.h>
#include <stdarg.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

extern long numofthreads;

//! @typedef 
typedef unsigned (*HashFunction)(void *data);

//! @typedef 
typedef void (*VisitFunction)(void *data);

//! @struct _Data
//! @brief Estructura de datos que guarda la cache.
//! @var val - char * : valor.
//! @var key - char : clave.
//! @var mode - int : modo (bin o text)
//! @var vlen - int : longitud del valor.
struct _Data {
  char *val;
  char *key;
  int mode;
  unsigned int vlen;
};

//! @typedef 
typedef struct _Data *Data;

void config_mutex(pthread_mutex_t* mtx);
void config_recursive_mutex(pthread_mutex_t* mtx);
//! @brief Desalojo de memoria (explicar).
//!
void release_memory();

//! @brief Función que maneja de manera correcta el intento de 
//! alocar memoria en la memcached. Intenta realizar un malloc, 
//! y en caso de que no consiga la memoria pedida, llama a la función
//! que realiza un procedimiento de desalojo de la caché.
//! Esto lo repite una cantidad máxima de intentos.
//!
//! @param[in] size - size_t : cantidad de memoria a alocar.
//! @param[out] ptr - void** : variable que apuntará a la memoria conseguida.
//! @return int - int : 0 si pudo allocar memoria, -1 en caso contrario.
int try_malloc(size_t size, void** ptr);

//! @brief Funcion de hash para strings.
//!
//! Propuesta por Kernighan & Ritchie en "The C Programming Language (Second Ed.)".
//! 
//! @param[in] s - char * : string a hashear.
unsigned KRHash(char *s);

//! @brief Crea una estructura tipo Data.
//!
//! @param[in] val - char * : valor.
//! @param[in] key - char * : clave.
//! @param[in] vlen - int : longitud del valor
//! @return data - Data: dato creado.
Data create_data(char* val, char* key, int mode, unsigned int vlen);

//! @brief Destruye el dato.
//!
//! @param[in] data - Data.
void destroy_data(Data data);

//! @brief Crea una copia del dato.
//!
//! @param[in] data - Data.
//! @return newData - Data : copia del dato original.
Data copy_data(Data data);

//! @brief Compara dos datos.
//!
//! Dos datos se consideran iguales si tienen la misma clave.
//!
//! @param[in] key1 - char * : primer valor a comparar.
//! @param[in] key2 - char * : segundo valor a comparar.
//! @return val - int : 1 si son iguales, 0 en caso contrario.
int compare_data(char* key1, char* key2);

//! @brief Imprime en pantalla un dato.
//!
//! @param[in] data - Data : dato a imprimir.
void print_data(Data data);

#endif /** __UTILS_H__ */