#include "epoll.h"

void epoll_ctl_add(int epfd, int fd, struct epoll_event ev) {
  /* configuración de sockets para eventos de lectura (EPOLLIN) */
	ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
	ev.data.fd = fd;
	
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
		perror("epoll_ctl: listen_sock");
		exit(EXIT_FAILURE);
	}
}

void epoll_ctl_mod(int epfd, int fd, struct epoll_event ev) {
  /* configuración de sockets para eventos de lectura (EPOLLIN) */
	ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
	ev.data.fd = fd;
	
	if (epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev) == -1) {
		perror("epoll_ctl: listen_sock");
		exit(EXIT_FAILURE);
	}
}