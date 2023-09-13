#ifndef __BIN_PARSER_H
#define __BIN_PARSER_H

#define _GNU_SOURCE /* strchrnul */

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include "command.h"
#include "common.h"

Cache cache;
ConcurrentQueue queue;

/*
 * creo una estructura para el cliente en bianrio
 * despues vemos si la usamos o no
*/

struct _BinClient {
	enum cursor cursor;
	enum code code;
  char* val;
  char* key;
  uint32_t lval;
  uint32_t lkey;
};

typedef struct _BinClient* BinClient;

/* 
 * podemos hacer una estructura que representa un cursor para ir viendo
 * en que parte del buffer estamos
*/

enum cursor {
	COMMAND = 0,
	KEY_SIZE = 1,
	KEY = 2,
	VALUE_SIZE = 3,
	VALUE = 4,
	FINISH = 5,
};

/* Macro interna */
#define READ(fd, buf, n) ({						\
	int rc = read(fd, buf, n);					\
	if (rc < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))	\
		return 0;						\
	if (rc <= 0)							\
		return -1;						\
	rc; })

int bin_parser(BinClient binClient);

int bin_consume(int fd, BinClient binClient);

void bin_handle();

#endif