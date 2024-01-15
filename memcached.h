#ifndef __MEMCACHED_H__
#define __MEMCACHED_H__

#include "sock.h"
#include "parser.h"

Cache cache;
ConcurrentQueue queue;

#define MAX_EVENTS 100
#define TEXT_MODE 0
#define BIN_MODE 1

eventloopData* create_evloop(int epollfd, int text_sock, int bin_sock, int id);

void limit_mem();

void init_server(int text_sock, int bin_sock);

void* server();

void handle_conn(CData client, int fd);

#endif