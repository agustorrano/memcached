#ifndef __BIN_PARSER_H
#define __BIN_PARSER_H

#define _GNU_SOURCE /* strchrnul */

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
// #include "command.h"
#include "common.h"

//Cache cache;
//ConcurrentQueue queue;

/*
 * creo una estructura para el cliente en bianrio
 * despues vemos si la usamos o no
*/

typedef struct _BinClient* BinClient;

/* Macro interna */
#define READ(fd, buf, n) ({						\
	int rc = read(fd, buf, n);					\
	if (rc < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))	\
		return 0;						\
	if (rc <= 0)							\
		return -1;						\
	rc; })

int bin_parser(int fd, char *buf, char *toks[], int lens[]);

int bin_consume(int fd);

void bin_handle();

#endif