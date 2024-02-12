#ifndef __BIN_PARSER_H
#define __BIN_PARSER_H

#include "command.h"
#include "epoll.h"

#define STATE_COMMAND 0
#define STATE_KSIZE 1
#define STATE_KEY 2
#define STATE_VSIZE 3
#define STATE_VALUE 4


/* Macro interna */
#define READ(fd, buf, n) ({						\
	int rc = read(fd, buf, n);	\
	if (rc < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))	\
		rc = 0;						\
	if (rc <= 0)							\
		rc = -1;						\
	rc; })

//! @brief Ejecuta en la memcached el pedido del cliente.
//!
//! @param[in] client - ClientData.  
//! @param[in] command - enum code.  
//! @param[in] toks - char*[].  
//! @param[in] lens - int[].  
int handler(enum code command, char** toks, unsigned lens[2], int mode, int threadId, int fd);

int parse_command(CBinData client, ListeningData ld);

int parse_ksize(CBinData client, ListeningData ld);

int parse_key(CBinData client, ListeningData ld);

int parse_vsize(CBinData client, ListeningData ld);

int parse_value(CBinData client, ListeningData ld);

//! @brief Consume la entrada del fd del cliente, utilizando la macro READ.
//!
//! @param[in] client - ClientData.  
//! @param[out] buf - char* : Buffer donde se almacenará lo consumido.
//! @param[in] blen - int.  
int bin_consume(ListeningData ld);


//! @brief Escribe en el socket del cliente la respuesta del pedido.
//!
//! @param[in] res - enum code : comando respuesta. 
//! @param[in] buf - char*.  
//! @param[in] blen - int. 
//! @param[in] fd - int : fd del socket.  
int write_bin(enum code res, char* buf, unsigned int blen, int fd);

#endif
