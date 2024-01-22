#include "epoll.h"
#include "parser.h"

eventloopData* create_evloop(int epollfd, int text_sock, int bin_sock, int id, int nproc) {
	eventloopData* info = malloc(sizeof(eventloopData));
	info->bin_sock = bin_sock;
	info->text_sock = text_sock;
	info->epfd = epollfd;
	info->id = id;
	info->nproc = nproc;
	statsTh[id] = create_stats();
	return info;
}

ClientData* create_clientData(int fd, int mode){
	ClientData* client = malloc(sizeof(ClientData));
	client->fd = fd;
	client->mode = mode;
	return client;
}

void epoll_ctl_add(int epfd, struct epoll_event ev, int fd, int mode) {
	ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
	//ev.data.fd = fd;
	ClientData* client = create_clientData(fd, mode);
	ev.data.ptr = client;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
		perror("epoll_ctl_add: listen_sock");
		exit(EXIT_FAILURE);
	}
}

void epoll_ctl_mod(int epfd, struct epoll_event ev, int fd, int mode) {
	ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
	//ev.data.fd = fd;
	ClientData* client = create_clientData(fd, mode);
	ev.data.ptr = client;
	if (epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev) == -1) {
		perror("epoll_ctl_mod: listen_sock");
		exit(EXIT_FAILURE);
	}
}