#ifndef __MEMCACHED_H__
#define __MEMCACHED_H__

#include "sock.h"
#include "parser.h"

#define MAX_EVENTS 100

//! @brief Limita el uso de memoria del servidor,
//! mediante el uso de setrlimit
void limit_mem();


//! @brief Crea la instancia epoll y agrega los dos sockets de
//! escucha correspondientes a los distintos protocolos. Crea estructuras
//! necesarias, y luego lanza un hilo por cada hilo de hardware disponible.
//!
//! @param[in] text_sock - int: fd asociado al socket de texto.
//! @param[in] bin_sock - int: fd asociado al socket binario.
void init_server(int text_sock, int bin_sock);


//! @brief Función principal de la memcached.
//! En ella, los hilos quedan bloqueados en epoll_wait hasta que haya
//! eventos que manejar. En este caso, diferencian si son eventos en los
//! sockets principales, lo cual significa que un cliente nuevo intenta
//! conectarse al servidor, o si son eventos de sockets de clientes, los
//! cuales tienen nuevos pedidos que atender.
//!
//! @param[in] arg - void*: identificador del thread.
void* server(void* arg);

//! @brief Función que comienza con el pedido del cliente.
//! Llama a la función consumir correspondiente, según el protocolo.
//! Luego, verifica si el cliente sigue conectado, en cuyo caso, 
//! lo agrega nuevamente a la epoll, hasta que tenga nuevos pedidos.
//! @param[in] client - ClientData: estructura que guarda información del cliente a manejar.
void handle_conn(ListeningData client);

#endif