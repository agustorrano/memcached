#include "epoll.h"
#include "parser.h"

eventloopData create_evloop(int epollfd, int text_sock, int bin_sock) {
	eventloopData info = malloc(sizeof(eventloopData));
	info->bin_sock = bin_sock;
	info->text_sock = text_sock;
	info->epfd = epollfd;
	return info;
}

ClientData create_clientData(int fd, int mode, int id){
	ClientData client = malloc(sizeof(ClientData));
	client->fd = fd;
	client->mode = mode;
	client->threadId = id;
	return client;
}

void epoll_ctl_add(int epfd, struct epoll_event ev, int fd, int mode, int id) {
	ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
	//ev.data.fd = fd;
	ClientData client = create_clientData(fd, mode, id);
	ev.data.ptr = client;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
		perror("epoll_ctl_add: listen_sock");
		exit(EXIT_FAILURE);
	}
}

void epoll_ctl_mod(int epfd, struct epoll_event ev, int fd, int mode, int id) {
	ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
	//ev.data.fd = fd;
	ClientData client = create_clientData(fd, mode, id);
	ev.data.ptr = client;
	if (epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev) == -1) {
		perror("epoll_ctl_mod: listen_sock");
		exit(EXIT_FAILURE);
	}
}