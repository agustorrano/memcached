#ifndef PARSER_H
#define PARSER_H

#define _GNU_SOURCE /* strchrnul */

#include "utils.h"
#include "common.h"
#include "command.h"

#define MAX_TOKS_T 3
#define MAX_TOKS_B 2

/* Macro interna */
#define READ(fd, buf, n) ({						\
	int rc = read(fd, buf, n);					\
	if (rc < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))	\
		return 0;						\
	if (rc <= 0)							\
		return -1;						\
	rc; })

enum code text_parser(char *buf, char *toks[MAX_TOKS_T], int lens[MAX_TOKS_T]);

int text_consume(char buf[2024], int fd, int blen);

void text_handle(enum code command, char* toks[MAX_TOKS_T], int lens[MAX_TOKS_T]);

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