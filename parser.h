#ifndef __PARSER_H
#define __PARSER_H

#include "common.h"
#include "command.h"
#include "epoll.h"

#define MAX_TOKS 2
// #define MAX_TOKS_B 2 si son iguales podrian ser la misma

/* Macro interna */
#define READ(fd, buf, n) ({						\
	int rc = read(fd, buf, n);				\
	if (rc < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))	\
		rc = 0;						\
	if (rc <= 0)							\
		rc = -1;						\
	rc; })

void handler(ClientData client, enum code command, char* toks[MAX_TOKS], int lens[MAX_TOKS]);

enum code text_parser(char *buf, char *toks[MAX_TOKS], int lens[MAX_TOKS]);

enum code bin_parser (char *buf, char *toks[], int lens[]);

int text_consume(ClientData client, char buf[2024], int blen, int size);

int bin_consume(ClientData client, char buf[2024], int blen, int size);

void write_text(enum code res, char* buf, int blen, int fd);

void write_bin(enum code res, char* buf, int blen, int fd);

#endif