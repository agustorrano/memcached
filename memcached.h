#ifndef __MEMCACHED_H__
#define __MEMCACHED_H__

#include "sock.h"
#include "common.h"
#include "text_parser.h"
#include "bin_parser.h"
#include "hashtable.h"
#include "concqueue.h"
#include "utils.h"

#define MAX_EVENTS 100

int numofthreads;

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

void server(eventloopData* info);
#endif
