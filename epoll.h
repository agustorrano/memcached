#ifndef __EPOLL_H__
#define __EPOLL_H__

#include "utils.h"

typedef struct eventloop_data {
	int epfd; // file descriptor para epoll
	int text_sock;
	int bin_sock;
	int id;
  int nproc;
} eventloopData;

typedef struct client_data {
	int mode;
	int fd;
} ClientData;

eventloopData* create_evloop(int epollfd, int text_sock, int bin_sock, int id, int nproc);

ClientData* create_clientData(int fd, int mode);

void epoll_ctl_add(int epfd, struct epoll_event ev, int fd, int mode);

void epoll_ctl_mod(int epfd, struct epoll_event ev, int fd, int mode);
#endif