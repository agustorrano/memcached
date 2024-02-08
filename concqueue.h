#ifndef __COLA_CONCURRENTE_H
#define __COLA_CONCURRENTE_H

#include "list.h"

//! @struct _Queue
//! @brief Estructura que representa una cola.
//! @var first - DNode* : puntero al primer nodo de la cola.
//! @var last - DNode* : puntero al último nodo de la cola.
struct _Queue {
  DNode* first;
  DNode* last;
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


//! @brief Destruye la cola.
//!
//! @param[in] queue - Queue.
void destroy_queue(Queue queue);


//! @brief Elimina un dato de la cola.
//!
//! Si el dato no está presente, no hace nada.
//!
//! @param[in] queue - Queue.  
//! @param[in] key - char *: dato a eliminar.
void delete_in_queue(Queue queue, DNode* node);


//! @brief Elimina el primer dato de la cola.
//!
//! Si la cola está vacía devuelve NULL.
//!
//! @param[in] queue - Queue. 
//! @return ret - char * : primer elemento de la cola.
char* pop(Queue queue);


void update_queue(Queue queue, DNode* node);

void lock_queue(ConcurrentQueue cq);

void unlock_queue(ConcurrentQueue cq);

void push_queue(ConcurrentQueue cq, Data data, int* flag_enomem);


//! @brief Inicializa una estructura tipo ConcurrentQueue.
//! 
//! Permite concurrencia.
//!
//! @param[in] concurrentQueue - ConcuurentQueue.
void init_concurrent_queue(ConcurrentQueue concurrentQueue);


//! @brief Determina si la cola está vacía.
//!
//! @param[in] concurrentQueue - ConcurrentQueue.
//! @return val - int: 1 si la cola está vacía, 0 en caso contrario.
int empty_concurrent_queue(ConcurrentQueue concurrentQueue);


//! @brief Destruye la cola concurrente.
//!
//! @param[in] concurrenQueue - ConcurrentQueue.
void destroy_concurrent_queue(ConcurrentQueue concurrentQueue);


//! @brief Elimina un dato de la cola concurrente.
//! 
//! @param[in] concurrentQueue - ConcurrentQueue.
//! @param[in] key - char* : dato a buscar.
void delete_in_concurrent_queue(ConcurrentQueue concurrentQueue, DNode* node);


//! @brief Elimina el primer elemento de la cola concurrente.
//! 
//! @param[in] concurrentQueue - ConcurrentQueue.
//! @return ret - char * : primer elemento de la cola.
char* pop_concurrent_queue(ConcurrentQueue concurrentQueue);

void update_concurrent_queue(ConcurrentQueue cq, DNode* node);

#endif 