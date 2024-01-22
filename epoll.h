#ifndef __EPOLL_H__
#define __EPOLL_H__

#include "utils.h"

//! @struct _eventloop_data
//! @brief Estructura de datos que maneja el epoll.
//! @var epfd - int : file descriptor para epoll.
//! @var text_sock - int : file descriptor para el socket de escucha en modo texto.
//! @var bin_sock - int : file descriptor para el socket de escucha en modo binario.
struct _eventloop_data {
	int epfd;
	int text_sock;
	int bin_sock;
} ;

//! @typedef
typedef struct _eventloop_data* eventloopData;

//! @struct _client_data
//! @brief Estructura de datos que guardan los fd que monitorea epoll.
//! @var fd - int : file descriptor del socket que se está monitoreando.
//! @var mode - int : modo en el que se conecta el cliente. 
//! @var threadId - int : identificador del thread que maneja al cliente.
//!
//! Todos los fd que monitorea la instancia epoll del server, guardan en
//! data.ptr esta estructura. Si el fd corresponde a un cliente, el modo 
//! se extiende a 0 (texto) o 1(binario). Si el fd corresponde al text_sock, 
//! o bin_sock, tanto mode como threadId serán -1.
//! 
struct _client_data {
	int mode;
	int fd;
	int threadId;
};

//! @typedef
typedef struct _client_data* ClientData;


//! @brief Crea una estructura tipo eventloopData.
//!
//! @param[in] epollfd - int : fd para epoll.
//! @param[in] text_sock - int : fd para el socket de escucha en modo texto.
//! @param[in] bin_sock - int : fd para el socket de escucha en modo binario.
//! @return info - eventloopData : estructura creada.
eventloopData create_evloop(int epollfd, int text_sock, int bin_sock);

//! @brief Crea una estructura tipo ClientData.
//!
//! @param[in] fd - int : fd correspondiente al cliente.
//! @param[in] mode - int : modo en el que se conecta el cliente. 
//! @param[in] id - int : identificador del thread que maneja al cliente.
//! @return info - eventloopData : estructura creada.
ClientData create_clientData(int fd, int mode, int id);

void epoll_ctl_add(int epfd, struct epoll_event ev, int fd, int mode, int id);

void epoll_ctl_mod(int epfd, struct epoll_event ev, int fd, int mode, int id);
#endif