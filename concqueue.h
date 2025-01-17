#ifndef __COLA_CONCURRENTE_H
#define __COLA_CONCURRENTE_H

#include "utils.h"

//! @struct _DNode
//! @brief Estructura que representa nodo de una lista doblemente enlazada.
//! @var key - char * : clave del dato.
//! @var next - _DNode* : puntero al siguiente nodo de la lista.
//! @var prev - _DNode* : puntero al nodo anterior de la lista.
typedef struct _DNode
{
  char *key;
  struct _DNode *next;
  struct _DNode *prev;
} DNode;

//! @struct _Queue
//! @brief Estructura que representa una cola.
//! @var first - DNode* : puntero al primer nodo de la cola.
//! @var last - DNode* : puntero al último nodo de la cola.
struct _Queue
{
  DNode *first;
  DNode *last;
};

//! @typedef
typedef struct _Queue *Queue;

//! @struct _ConcurrentQueue
//! @brief Estructura que representa una cola concurrente.
//! @var queue - Queue.
//! @var mutex - pthread_mutex_t : mutex de la cola.
struct _ConcurrentQueue
{
  Queue queue;
  pthread_mutex_t mutex;
};

//! @typedef
typedef struct _ConcurrentQueue *ConcurrentQueue;

//! @brief Crea una cola.
//!
//! @return queue - Queue: cola creada.
Queue create_queue();

//! @brief Determina si la cola está vacía.
//!
//! @param[in] queue - Queue.
//! @return val - int: 1 si la cola está vacía, 0 en caso contrario.
int empty_queue(Queue queue);

//! @brief Busca un dato en la cola.
//!
//! @param[in] queue - Queue.
//! @param[in] key - char *: dato que se quiere encontrar.
//! @return node - DNode* : dato encontrado (o NULL).
DNode *search_queue(Queue queue, char *key);

//! @brief Inserta un dato en la cola concurrente.
//!
//! @param[in] queue - Queue
//! @param[in] newNode - DNode* : nodo con el dato a insertar
void push_queue(Queue queue, DNode *newNode);

//! @brief Elimina el primer dato de la cola.
//!
//! Si la cola está vacía devuelve NULL.
//!
//! @param[in] queue - Queue.
//! @return ret - char * : primer elemento de la cola.
char *pop(Queue queue);

//! @brief Destruye la cola.
//!
//! @param[in] queue - Queue.
void destroy_queue(Queue queue);

//! @brief Elimina y destruye un dato de la cola.
//!
//! Si el dato no está presente, no hace nada.
//!
//! @param[in] queue - Queue.
//! @param[in] key - char *: dato a eliminar.
void delete_in_queue(Queue queue, char *key);

//! @brief Inicializa una estructura tipo ConcurrentQueue.
//!
//! @param[in] concurrentQueue - ConcuurentQueue.
void init_concurrent_queue(ConcurrentQueue concurrentQueue);

//! @brief Destruye la cola concurrente.
//!
//! @param[in] concurrenQueue - ConcurrentQueue.
void destroy_concurrent_queue(ConcurrentQueue concurrentQueue);

//! @brief Elimina un dato de la cola concurrente.
//!
//! @param[in] concurrentQueue - ConcurrentQueue.
//! @param[in] key - char* : dato a buscar.
void delete_in_concurrent_queue(ConcurrentQueue concurrentQueue, char *key);

//! @brief Función utilizada para implementar LRU.
//!
//! Verifica que el nodo esté en la cola, si es así, lo remueve
//! de su lugar. Si no está, lo crea. Luego realiza un push.
//!
//! Permite concurrencia.
//!
//! @param[in] cqueue - ConcurrentQueue
//! @param[in] key - char * : dato a insertar.
//! @param[out] flag_enomem - int* : bandera para informar que no se pudo allocar memoria.
void update_queue(ConcurrentQueue cqueue, char *key, int *flag_enomem);

//! @brief Función utilizada para implementar LRU.
//!
//! La función es llamada únicamente si el nodo está en la cola.
//! Remueve el nodo de la misma, y devuelve el siguiente.
//!
//! @param[in] queue - Queue
//! @param[in] node - DNode * : nodo que se quiere sacar.
//! @param[out] flag - int : bandera para informar si se debe o no destruir el nodo.
DNode *remove_from_queue(Queue queue, DNode *node, int flag);

#endif