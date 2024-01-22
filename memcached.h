#ifndef __MEMCACHED_H__
#define __MEMCACHED_H__

#include "sock.h"
#include "parser.h"

#define MAX_EVENTS 100

void limit_mem();

void init_server(int text_sock, int bin_sock);

void* server(void* arg);

void handle_conn(ClientData client);

#endif