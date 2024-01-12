#ifndef __BIN_PARSER_H
#define __BIN_PARSER_H

#define _GNU_SOURCE /* strchrnul */

#include "command.h"
#include "common.h"
#include "utils.h"

#define MAX_TOKS_B 2

/*
 * creo una estructura para el cliente en bianrio
 * despues vemos si la usamos o no
 */

typedef struct _BinClient *BinClient;

/* Macro interna */
#define READ(fd, buf, n) ({						\
	int rc = read(fd, buf, n);					\
	if (rc < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))	\
		return 0;						\
	if (rc <= 0)							\
		return -1;						\
	rc; })

enum code bin_parser (char *buf, char *toks[], int lens[]);

int bin_consume(int fd);

//void bin_handle(enum code command, char *toks[MAX_TOKS_B], int lens[MAX_TOKS]);

#endif