#ifndef __UTILS_H__
#define __UTILS_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>

#define CAPACIDAD_INICIAL_TABLA 10

//! @typedef 
typedef unsigned (*HashFunction)(void *data);

//! @typedef 
typedef void (*VisitFunction)(void *data);

//! @struct _Data
//! @brief Estructura de datos que guarda la cache.
//! @var val - char * : valor.
//! @var key - char : clave.
//!
struct _Data {
  char *val;
  char *key;
};

//! @typedef 
typedef struct _Data *Data;


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
//! @return data - Data: dato creado.
Data create_data(char* val, char* key);

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