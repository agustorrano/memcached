#include "epoll.h"
#include "parser.h"

eventloopData create_evloop(int epollfd, int text_sock, int bin_sock) {
	eventloopData info;
	if (try_malloc(sizeof(struct _eventloop_data), (void*)&info) == -1) {
    errno = ENOMEM;
		perror("Initializing Structs");
		exit(EXIT_FAILURE);
	}
	info->bin_sock = bin_sock;
	info->text_sock = text_sock;
	info->epfd = epollfd;
	return info;
}

ClientData create_clientData(int fd, int mode, int id, int* flag_enomem){
	ClientData client;
	if (try_malloc(sizeof(struct _client_data), (void*)&client) == -1) {
		*flag_enomem = 1;
		return NULL;
	}
	client->fd = fd;
	client->mode = mode;
	client->threadId = id;
	client->lenBuf = 0;
	client->buf = NULL;
	return client;
}

void epoll_ctl_add(int epfd, struct epoll_event ev, int fd, int mode, int id) {
	if (mode == -1)
		ev.events = EPOLLIN | EPOLLET | EPOLLEXCLUSIVE;
	else
		ev.events = EPOLLIN | EPOLLONESHOT;
	ClientData client = create_clientData(fd, mode, id);
	ev.data.ptr = client;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
		perror("epoll_ctl_add: listen_sock");
		exit(EXIT_FAILURE);
	}
}

void epoll_ctl_mod(int epfd, struct epoll_event ev, ClientData client) {
	ev.events = EPOLLIN | EPOLLONESHOT;
	ev.data.ptr = client;
	if (epoll_ctl(epfd, EPOLL_CTL_MOD, client->fd, &ev) == -1) {
		perror("epoll_ctl_mod: listen_sock");
		exit(EXIT_FAILURE);
	}
}