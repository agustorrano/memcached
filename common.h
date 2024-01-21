#ifndef __COMMON_H
#define __COMMON_H

#include "log.h"

enum code {
	PUT = 11,
	DEL = 12,
	GET = 13,

	STATS = 21,

	OK = 101,
	EINVALID = 111,
	ENOTFOUND = 112,
	EBINARY = 113,
	EBIG = 114,
	EUNK = 115,
	EOOM = 116,
};

/* valida el pedido */
int valid_rq(int code);

static const in_port_t mc_lport_text = 8888;
static const in_port_t mc_lport_bin  = 8889;

static inline void quit(char *s)
{
	perror(s);
	exit(1);
}

#define STATIC_ASSERT(p)			\
	int _ass_ ## __LINE__ [(!!(p)) - 1];

const char * error_str(enum code e);
const char * code_str(enum code e);

#endif