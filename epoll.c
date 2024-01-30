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

CTextData create_text_client(int fd, int id) {
	CTextData client;
	try_malloc(sizeof(struct _client_text_data), (void*)&client);
	client->fd = fd;
	client->threadId = id;
	client->buf = NULL;
	client->lenBuf = 0;
	return client;
}

CBinData create_bin_client(int fd, int id) {
	CBinData client;
	try_malloc(sizeof(struct _client_bin_data), (void*)&client);
	client->fd = fd;
	client->threadId = id;
	client->toks = NULL;
	client->lens = NULL;
	client->command = NULL;
	client->state = 0;
	client->cursor = 0;
	return client;
}

ListeningData create_lclient(int fd, int mode) {
	ListeningData client;
	try_malloc(sizeof(struct _listening_data), (void*)&client);
	client->fd = fd;
	client->mode = mode;
	return client;
}

/*
ClientData create_clientData(int fd, int mode, int id){
	ClientData client;
	if (try_malloc(sizeof(struct _client_data), (void*)&client) == -1) { return NULL; }
	client->fd = fd;
	client->mode = mode;
	client->threadId = id;
	client->lenBuf = 0;
	client->buf = NULL;
	return client;
} */

void epoll_ctl_add(int epfd, struct epoll_event ev, int fd, int mode, int id) {
	//ClientData client = create_clientData(fd, mode, id);
	CTextData textClient;
	CBinData binClient;

	if (mode == TEXT_MODE) {
		textClient = create_text_client(fd, id);
		ev.data.ptr = textClient;
		if (textClient == NULL && mode != -1) {
			if (write(fd, "EOOM\n", 4) < 0) {
    	  perror("Error al escribir en el socket");
    	  exit(EXIT_FAILURE);
			}
			close(fd);
			return;
		}
		else if (textClient == NULL && mode == -1) { // no pudimos agregar los listening sockets
			errno = ENOMEM;
			perror("Initializing Structs");
			exit(EXIT_FAILURE);
		}
	}

	else {
		binClient = create_bin_client(fd, id);
		ev.data.ptr = binClient;
		if (binClient == NULL && mode != -1) {
			if (write(fd, "EOOM\n", 4) < 0) {
    	  perror("Error al escribir en el socket");
    	  exit(EXIT_FAILURE);
			}
			close(fd);
			return;
		}
		else if (binClient == NULL && mode == -1) { // no pudimos agregar los listening sockets
			errno = ENOMEM;
			perror("Initializing Structs");
			exit(EXIT_FAILURE);
		}
	}

	//ev.data.ptr = client;
	if (mode == -1)
		ev.events = EPOLLIN | EPOLLET | EPOLLEXCLUSIVE;
	else
		ev.events = EPOLLIN | EPOLLONESHOT;

	if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
		perror("epoll_ctl_add: listen_sock");
		exit(EXIT_FAILURE);
	}
}


void epoll_ctl_mod(int epfd, struct epoll_event ev, ListeningData client) {
	ev.events = EPOLLIN | EPOLLONESHOT;
	ev.data.ptr = client;
	if (epoll_ctl(epfd, EPOLL_CTL_MOD, client->fd, &ev) == -1) {
		perror("epoll_ctl_mod: listen_sock");
		exit(EXIT_FAILURE);
	}
}