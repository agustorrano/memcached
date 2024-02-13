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

//! @brief Maneja los pedidos de los clientes.
//!
//! @param[in] command - enum code : comando ejecutado.
//! @param[in] toks - char** : lugar donde se guardan la clave y el valor.
//! @param[in] lens - unsigned[] : lugar donde se guardan las longitudes de la
//! clave y del valor.
//! @param[in] mode - int : modo en el que se hace el pedido.
//! @param[in] threadId - int : id del hilo.
//! @param[in] fd - int : file descriptor.
//! @return -1 en caso de error, 0 si no.
int handler(enum code command, char **toks, unsigned lens[2], int mode, int threadId, int fd);

//! @brief Consume y parsea el comando.
//!
//! @param[in] client - CBinData.
//! @param[in] ld - ListeningData.
//! @return -1 en caso de error, 0 si no.
int parse_command(CBinData client, ListeningData ld);

//! @brief Consume y parsea la longitud de la clave.
//!
//! @param[in] client - CBinData.
//! @param[in] ld - ListeningData.
//! @return -1 en caso de error, 0 si no.
int parse_ksize(CBinData client, ListeningData ld);

//! @brief Consume y parsea la clave.
//!
//! @param[in] client - CBinData.
//! @param[in] ld - ListeningData.
//! @return -1 en caso de error, 0 si no.
int parse_key(CBinData client, ListeningData ld);

//! @brief Consume y parsea la longitud del valor.
//!
//! @param[in] client - CBinData.
//! @param[in] ld - ListeningData.
//! @return -1 en caso de error, 0 si no.
int parse_vsize(CBinData client, ListeningData ld);

//! @brief Consume y parsea el valor.
//!
//! @param[in] client - CBinData.
//! @param[in] ld - ListeningData.
//! @return -1 en caso de error, 0 si no.
int parse_value(CBinData client, ListeningData ld);

//! @brief Consume por partes la entrada del fd del cliente.
//!
//! @param[in] ld - ListeningData.
//! @return -1 en caso de que se cerró la conexión, 1 si el pedido está incompleto, 0 si no
int bin_consume(ListeningData ld);

//! @brief Escribe en el socket del cliente la respuesta del pedido.
//!
//! @param[in] res - enum code : comando respuesta.
//! @param[in] buf - char*.
//! @param[in] blen - int.
//! @param[in] fd - int : fd del socket.
//! @return -1 en caso de que se cerró la conexión, 0 si no.
int write_bin(enum code res, char *buf, unsigned int blen, int fd);

#endif
