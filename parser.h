#ifndef __PARSER_H
#define __PARSER_H

#include "common.h"
#include "command.h"
#include "epoll.h"

#define MAX_TOKS_T 2
#define MAX_TOKS_B 2 // si son iguales podrian ser la misma
#define TEXT_MODE 0
#define BIN_MODE 1

extern Cache cache;
extern ConcurrentQueue queue;
extern Stats* statsTh;

/* Macro interna */
#define READ(fd, buf, n) ({						\
	int rc = read(fd, buf, n);				\
	if (rc < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))	\
		rc = 0;						\
	if (rc <= 0)							\
		rc = -1;						\
	rc; })

void text_handle(eventloopData* infoTh, enum code command, char* toks[MAX_TOKS_T], int lens[MAX_TOKS_T], int fd);

enum code text_parser(char *buf, char *toks[MAX_TOKS_T], int lens[MAX_TOKS_T]);

int text_consume(eventloopData* infoTh, char buf[2024], int fd, int blen, size_t size);

enum code bin_parser (char *buf, char *toks[], int lens[]);

int bin_consume(eventloopData* infoTh, char* buf, int fd, int blen, size_t size);

//void bin_handle(enum code command, char *toks[MAX_TOKS_B], int lens[MAX_TOKS]);

#endif