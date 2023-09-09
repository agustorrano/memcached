#ifndef __TEXT_PARSER_H
#define __TEXT_PARSER_H

#define _GNU_SOURCE /* strchrnul */

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include "common.h"
#include "command.h"

#define MAX_TOKS 3

Cache cache;
ConcurrentQueue queue;

/* Macro interna */
#define READ(fd, buf, n) ({						\
	int rc = read(fd, buf, n);					\
	if (rc < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))	\
		return 0;						\
	if (rc <= 0)							\
		return -1;						\
	rc; })

enum code text_parser(char *buf, char *toks[MAX_TOKS], int lens[MAX_TOKS]);

int text_consume(struct eventloop_data *evd, char buf[2024], int fd, int blen);

void text_handle(enum code command, char* toks[MAX_TOKS], int lens[MAX_TOKS]);

#endif