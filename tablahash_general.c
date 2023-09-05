#include "tablahash.h"

TablaHash tablahash_crear(unsigned capacidad, FuncionCopia copy, FuncionComparadora comp, 
                          FuncionDestructora destroy, FuncionHash hash) {

  TablaHash tabla = malloc(sizeof(struct _TablaHash));
  assert(tabla != NULL);
  tabla->elems = malloc(sizeof(GList) * capacidad);
  assert(tabla->elems != NULL);
  tabla->numElems = 0;
  tabla->capacidad = capacidad;
  tabla->destr = destroy;
  tabla->copy = copy;
  tabla->comp = comp;
  tabla->hash = hash;
  for (unsigned idx = 0; idx < capacidad; ++idx)
    tabla->elems[idx] = glist_crear();
  return tabla;
}

int tablahash_nelems(TablaHash tabla) { return tabla->numElems; }

int tablahash_capacidad(TablaHash tabla) { return tabla->capacidad; }

void tablahash_destruir(TablaHash tabla) {
  for (unsigned idx = 0; idx < tabla->capacidad; ++idx)
    glist_destruir(tabla->elems[idx], tabla->destr);
  free(tabla->elems);
  free(tabla);
  return;
}

 void tablahash_insertar(TablaHash tabla, void *dato) {
  unsigned idx = tabla->hash(dato) % tabla->capacidad;
  void* buscado = glist_buscar(tabla->elems[idx], dato, (FuncionComparadora)tabla->comp);
  if (buscado != NULL) { 
    buscado->valor = dato->valor;
  }
  tabla->numElems++;
  int loadfactor = (tabla->numElems * 100) / tabla->capacidad;
  if (loadfactor > 75) {
    tablahash_rehash(tabla);
    idx = tabla->hash(dato) % tabla->capacidad;
  }
  tabla->elems[idx] = glist_agregar_inicio(tabla->elems[idx], dato, tabla->copy);
  return;
} 

void* tablahash_buscar(TablaHash tabla, void *dato) {
  unsigned idx = tabla->hash(dato) % tabla->capacidad;
  return glist_buscar(tabla->elems[idx], dato, (FuncionComparadora)tabla->comp);
}

void tablahash_recorrer(TablaHash tabla, FuncionVisitante visit) {
  for (unsigned i = 0; i < tabla->capacidad; i++) {
    printf("%d", i); //indice del arreglo de casilleros
    glist_recorrer(tabla->elems[i], visit);
    printf("\n");
  }
  printf("\n");
  return;
}

/* void tablahash_rehash(TablaHash tabla) {
  unsigned capacidadvieja = tabla->capacidad;
  tabla->capacidad = tabla->capacidad * 2;
  
  //alocamos memoria para el nuevo arreglo
  GList *nuevoArreglo = malloc(sizeof(GList) * tabla->capacidad);
  assert(nuevoArreglo != NULL);

  // Inicializamos las casillas con datos nulos.
  for (unsigned idx = 0; idx < tabla->capacidad; ++idx) 
    nuevoArreglo[idx] = glist_crear();
  
  int key;
  for (unsigned idx = 0; idx < capacidadvieja; ++idx) {
    for (GNodo *node = tabla->elems[idx]; node != NULL; node = node->next) {
      key = tabla->hash(node->data) % tabla->capacidad;
      nuevoArreglo[key] = glist_agregar_inicio(nuevoArreglo[key], node->data, tabla->copy);
    }
  }

  //destruimos el viejo arreglo  
  for (unsigned idx = 0; idx < capacidadvieja; ++idx)
    if (!glist_vacia(tabla->elems[idx]))
      glist_destruir(tabla->elems[idx], tabla->destr);
  free(tabla->elems);

  //agregamos el nuevo arreglo a la tabla
  tabla->elems = nuevoArreglo;
  return;
} */

TablaHashConcurrente th_concurrente_init(TablaHashConcurrente th, int capacidad, FuncionCopiadora copy, FuncionComparadora comp, FuncionDestructora destr, FuncionVisit hash) {
  th->tabla = tablahash_crear(capacidad, (FuncionCopiadora*) copy, (FuncionComparadora*) comp, (FuncionDestructora*) destr, (FuncionVisit*) hash);
  pthread_mutex_init(&th->mutex, NULL);
}

void th_concurrente_insertar(TablaHashConcurrente th, void* dato) {
  pthread_mutex_lock(&th->mutex);
  tablahash_insertar(th->tabla, dato);
  pthread_mutex_unlock(&th->mutex);
  return;
}

void th_concurrente_liberar(TablaHashConcurrente th)
{
  tablahash_destruir(th->tabla);
  pthread_mutex_destroy(&th->mutex);
  return;
}

tablahash_eliminar(TablaHash tabla, void* dato) {
  unsigned idx = tabla->hash(dato) % tabla->capacidad;
  ??? buscado = glist_buscar(tabla->elems[idx], dato, (FuncionComparadora*)tabla->comp);
  if (buscado == NULL) { return; }
  tabla->numElems--;
  tabla->elems[idx] = glist_eliminar(tabla->elems[idx], dato);
  return;
};

void th_concurrente_eliminar(TablaHashConcurrente th, void* dato) {
  pthread_mutex_lock(&th->mutex);
  tablahash_eliminar(th->tabla, dato);
  pthread_mutex_unlock(&th->mutex);
  return;
}