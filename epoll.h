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

struct _listening_data {
	int mode;
	int fd;
	int threadId;
  void* client;
};
//! @typedef
typedef struct _listening_data* ListeningData;

struct _client_text_data {
	char* buf;
	int lenBuf;
};
typedef struct _client_text_data* CTextData;

struct _client_bin_data {
	//char** toks;
	uint8_t bytes[4];
	//unsigned lens[2];
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


ListeningData create_ld(int fd, int mode, int id, void* client);


CTextData create_textData();


CBinData create_binData();

//! @brief Agrega el fd, al conjunto gestionado por el epoll epfd.
//! Crea una estructura cliente, donde guarda los datos necesarios, y 
//! la almacena en el puntero del event.data.
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

/*
//! @struct _client_data
//! @brief Estructura de datos que guardan los fd que monitorea epoll.
//! @var fd - int : file descriptor del socket que se está monitoreando.
//! @var mode - int : modo en el que se conecta el cliente. 
//! @var threadId - int : identificador del thread que maneja al cliente.
//! @var buf - char* : buffer donde se almacenan pedidos incompletos.
//! @var lenBUf - int : longitud del buffer.
//!
//! Todos los fd que monitorea la instancia epoll del server, guardan en
//! data.ptr esta estructura. Si el fd corresponde a un cliente, el modo 
//! se extiende a 0 (texto) o 1(binario). Si el fd corresponde al text_sock, 
//! o bin_sock, tanto mode como threadId serán -1.

struct _client_data {
	int mode;
	int fd;
	int threadId;
	char* buf;
	int lenBuf;
};

//! @typedef
typedef struct _client_data* ClientData;


//! @brief Crea una estructura tipo ClientData.
//!
//! @param[in] fd - int : fd correspondiente al cliente.
//! @param[in] mode - int : modo en el que se conecta el cliente. 
//! @param[in] id - int : identificador del thread que maneja al cliente.
//! @return client - ClientData : estructura creada.
ClientData create_clientData(int fd, int mode, int id);


*/