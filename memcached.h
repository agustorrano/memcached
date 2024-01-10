#ifndef __MEMCACHED_H__
#define __MEMCACHED_H__

#include "hashtable.h"
#include "concqueue.h"
#include "sock.h"
#include "common.h"

#define MAX_EVENTS 100

int numofthreads;
Cache cache;
ConcurrentQueue queue;

typedef struct eventloop_data {
	int epfd; // file descriptor para epoll
	int id;
	int text_sock, bin_sock;
} eventloopData;

eventloopData* create_evloop(int epollfd, int text_sock, int bin_sock, int id);

#endif
