#ifndef __MEMCACHED_H__
#define __MEMCACHED_H__

#include "sock.h"
#include "parser.h"

Cache cache;
ConcurrentQueue queue;

#define MAX_EVENTS 100

/*
typedef struct eventloop_data {
	int epfd; // file descriptor para epoll
	int id;
	int text_sock, bin_sock;
} eventloopData;
*/

eventloopData* create_evloop(int epollfd, int text_sock, int bin_sock, int id);

void limit_mem();

void init_server(int text_sock, int bin_sock);

void server(void* arg);

#endif