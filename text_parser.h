#ifndef __TEXT_PARSER_H
#define __TEXT_PARSER_H

#define _GNU_SOURCE /* strchrnul */

#include "utils.h"
#include "common.h"
#include "command.h"

#define MAX_TOKS_T 3

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

#endif