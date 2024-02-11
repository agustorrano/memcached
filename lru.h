#ifndef __LRU_H
#define __LRU_H

#include "utils.h"

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

#endif