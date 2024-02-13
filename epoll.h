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

//! @struct _listening_data
//! @brief Estructura de datos del cliente.
//! Se crea cuando se acepta la conexión con el cliente, y se utilizará a 
//! lo largo de todo el servidor.
//! @var mode - int : file descriptor para epoll.
//! @var fd - int : file descriptor para el socket de escucha en modo texto.
//! @var threadId - int : file descriptor para el socket de escucha en modo binario.
//! @var client - void* : estructura del cliente (modo texto, binario, o NULL
//! si es un listening socket)
struct _listening_data {
	int mode;
	int fd;
	int threadId;
  void* client;
};

//! @typedef
typedef struct _listening_data* ListeningData;

//! @struct _client_text_data
//! @brief Estructura de datos del cliente en modo texto.
//! 
//! @var buf - char* : buffer donde se guardarán pedidos leidos
//! pero no atendidos.
//! @var lenBuf - int : longitud del buffer.
struct _client_text_data {
	char* buf;
	int lenBuf;
};
typedef struct _client_text_data* CTextData;

//! @struct _client_bint_data
//! @brief Estructura de datos del cliente en modo binario.
//! 
//! @var bytes - uint8_t [4] :
//! @var key - char* :
//! @var klen -	unsigned int:
//! @var value - char* :
//! @var vlen - unsigned int :
//! @var command - int :
//! @var state - int :
//! @var cursor - unsigned : 
struct _client_bin_data {
	uint8_t bytes[4];
	char* key;
	unsigned int klen;
	char* value;
	unsigned int vlen;
	int command;
	int state;
	unsigned cursor;
};

//! @typedef
typedef struct _client_bin_data* CBinData;


//! @brief Crea una estructura tipo eventloopData.
//!
//! @param[in] epollfd - int : fd para epoll.
//! @param[in] text_sock - int : fd para el socket de escucha en modo texto.
//! @param[in] bin_sock - int : fd para el socket de escucha en modo binario.
//! @return info - eventloopData : estructura creada.
eventloopData create_evloop(int epollfd, int text_sock, int bin_sock);


//! @brief Crea una estructura tipo ListeningData.
//!
//! @param[in] fd - int : file descriptor del socket de conexion (o listening socket).
//! @param[in] mode - int :  binario, texto, o -1 si es un listening socket.
//! @param[in] id - int : numero de thread que está manejando al cliente en este momento
//! o -1 si es un listening socket.
//! @param[in] client - void * : estructura del cliente (o NULL).
//! @return ld - ListeningData : estructura creada.
ListeningData create_ld(int fd, int mode, int id, void* client);


//! @brief Crea una estructura tipo CTextData.
//!
//! @return tclient - CTextData : estructura creada.
CTextData create_textData();


//! @brief Crea una estructura tipo CBinData.
//!
//! @return bclient - CBinData : estructura creada.
CBinData create_binData();


//! @brief Agrega el fd, al conjunto gestionado por el epoll epfd.
//! Crea una estructura cliente, donde guarda los datos necesarios, y 
//! la almacena en el puntero del event.data.
//!
//! @param[in] epfd - int : fd correspondiente a la instancia epoll.
//! @param[in] ev - struct epoll_event : estructura que especifica los eventos y datos asociados al fd.
//! @param[in] fd - int : fd a monitorear.
//! @param[in] mode - int : si es un cliente, es 1 o 0 según el protocolo.
//! si es uno de los sockets de escucha principal, es -1.
//! @param[in] id - int : si es un cliente, es el id del thread que lo acepta.
//! si es uno de los sockets de escucha principal, es -1.
void epoll_ctl_add(int epfd, struct epoll_event ev, int fd, int mode, int id);


//! @brief  Modifica la configuración de un descriptor de archivo existente en el 
//! conjunto gestionado por epoll.
//! Es necesaria solo en los sockets de los clientes (no en los listening sockets),
//! debido a la configuración con la cual agregamos a los fd: EPOLLIN | EPOLLONESHOT.
//! @param[in] epfd - int : fd correspondiente a la instancia epoll.
//! @param[in] ev - struct epoll_event : estructura que especifica los eventos y datos asociados al fd.
//! @param[in] client - ClientData : información del cliente.
void epoll_ctl_mod(int epfd, struct epoll_event ev, ListeningData client);

#endif