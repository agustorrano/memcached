#ifndef __TABLAHASH_H__
#define __TABLAHASH_H__
#include "glist.h"
#include "utils.h"
#include <pthread.h>

/**
 * Estructura principal que representa la tabla hash.
 */
struct _TablaHash {
  GList *elems;
  unsigned numElems;
  unsigned capacidad;
  FuncionDestructora destr;
  FuncionCopia copy;
  FuncionComparadora comp;
  FuncionHash hash;
};

typedef struct _TablaHash *TablaHash;

struct _TablaHashConcurrente {
  TablaHash tabla;
  pthread_mutex_t mutex;
}; 

typedef struct _TablaHashConcurrente *TablaHashConcurrente;



/**
 * Crea una nueva tabla hash vacia, con la capacidad dada.
 */
TablaHash tablahash_crear(unsigned capacidad, FuncionCopia copy,
                          FuncionComparadora comp, FuncionDestructora destr,
                          FuncionHash hash);

/*
 * Retorna el numero de elementos de la tabla.
 */
int tablahash_nelems(TablaHash tabla);

/*
 * Retorna la capacidad de la tabla.
 */
int tablahash_capacidad(TablaHash tabla);

/*
 * Destruye la tabla.
 */
void tablahash_destruir(TablaHash tabla);

/*
 * Inserta un dato en la tabla, utilizando la función hash.
 * Si el dato ya se encontraba, no hace nada. Dependiendo del factor de carga
 * aumenta la capacidad de la tabla cuando es necesario, utilizando la funcion
 * ReHash.
 */
void tablahash_insertar(TablaHash tabla, void *dato);

/*
 * Calcula el indice (segun la funcion hash) y verifica si el dato está en ese
 * casillero. Si esto ocurre retorna el dato de la tabla que coincida con el dato 
 * dado. Si el dato buscado no se encuentra en la tabla, devuelve NULL.
 */
void* tablahash_buscar(TablaHash tabla, void *dato);

/*
 * Recorre los casilleros de la tabla. Para cada casillero no vacío, recorre
 * cada elemento dentro de él (es decir, en su lista enlazada en caso de haber 
 * colisiones) y utiliza la función pasada como argumento. 
 */
void tablahash_recorrer(TablaHash tabla, FuncionVisitante visit);

/*
 * Se llama a esta funcion cuando el factor de carga de la tabla anterior era
 * mayor al recomendado. No se crea una nueva tabla, solamente un nuevo arreglo de
 * casilleros con mayor capacidad. Se copia la información del arreglo anterior al
 * nuevo, se guarda este nuevo arreglo en nuestra estructura TablaHash y se destruye
 * el anterior.
 */
void tablahash_rehash(TablaHash tabla);

TablaHashConcurrente th_concurrente_init(TablaHashConcurrente th, int capacidad, FuncionCopiadora copy, FuncionComparadora comp, FuncionDestructora destr, FuncionVisit hash);

void th_concurrente_insertar(TablaHashConcurrente th, void* dato);

void th_concurrente_liberar(TablaHashConcurrente th);



#endif /* __TABLAHASH_H__ */