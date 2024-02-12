#include "epoll.h"
#include "bin_parser.h"

eventloopData create_evloop(int epollfd, int text_sock, int bin_sock) {
	eventloopData info = malloc(sizeof(struct _eventloop_data));
	if (info == NULL) {
    errno = ENOMEM;
		perror("Initializing Structs");
		exit(EXIT_FAILURE);
	}
	info->bin_sock = bin_sock;
	info->text_sock = text_sock;
	info->epfd = epollfd;
	return info;
}

ListeningData create_ld(int fd, int mode, int id, void* client){
	ListeningData ld;
	if (try_malloc(sizeof(struct _listening_data), (void*)&ld) == -1) { return NULL; }
	ld->fd = fd;
	ld->mode = mode;
	ld->threadId = id;
	ld->client = client;
	return ld;
}

CTextData create_textData() {
	CTextData text_client;
	if (try_malloc(sizeof(struct _client_text_data), (void*)&text_client) == -1)
		return NULL;
	text_client->buf = NULL;
	text_client->lenBuf = 0;
	return text_client; 
}

CBinData create_binData() {
	CBinData bin_client;
	if (try_malloc(sizeof(struct _client_bin_data), (void*)&bin_client) == -1)
		return NULL;
	bin_client->state = STATE_COMMAND;
	return bin_client; // si dio error es NULL
}

void epoll_ctl_add(int epfd, struct epoll_event ev, int fd, int mode, int id) {
	CTextData tclient;
	CBinData bclient;
	ListeningData ld;
	if (mode == -1) {
		ld = create_ld(fd, mode, id, NULL);
		if (ld == NULL) {
			errno = ENOMEM;
			perror("Initializing Structs");
			exit(EXIT_FAILURE);
		}
		ev.data.ptr = ld;
	}
	else {
		if (mode == TEXT_MODE) {
			tclient = create_textData();
			if (tclient == NULL) {
				if (write(fd, "EOOM\n", 4) < 0) {
		  		perror("Error al escribir en el socket");
		  		exit(EXIT_FAILURE);
				}
				close(fd);
				return;
			}

			ListeningData ld = create_ld(fd, mode, id, (void*)tclient);
			if (ld == NULL) {
				if (write(fd, "EOOM\n", 4) < 0) {
				  perror("Error al escribir en el socket");
				  exit(EXIT_FAILURE);
				}
				close(fd);
				return;
			}
			ev.data.ptr = ld;
		}
		else  { // (mode == BIN_MODE)
			bclient = create_binData();
			if (bclient == NULL) {
				if (write(fd, "EOOM\n", 4) < 0) {
		  		perror("Error al escribir en el socket");
		  		exit(EXIT_FAILURE);
				}
				close(fd);
				return;
			}

			ListeningData ld = create_ld(fd, mode, id, (void*)bclient);
			if (ld == NULL) {
				if (write(fd, "EOOM\n", 4) < 0) {
				  perror("Error al escribir en el socket");
				  exit(EXIT_FAILURE);
				}
				close(fd);
				return;
			}
			ev.data.ptr = ld;
		}
	}
	
	if (mode == -1)
		ev.events = EPOLLIN | EPOLLET | EPOLLEXCLUSIVE;
	else
		ev.events = EPOLLIN | EPOLLONESHOT;

	int status = fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
	if (status == -1){
	  perror("calling fcntl");
		exit(EXIT_FAILURE);
	}
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
		perror("epoll_ctl_add: listen_sock");
		exit(EXIT_FAILURE);
	}
}


void epoll_ctl_mod(int epfd, struct epoll_event ev, ListeningData ld) {
	ev.events = EPOLLIN | EPOLLONESHOT;
	ev.data.ptr = ld;
	if (epoll_ctl(epfd, EPOLL_CTL_MOD, ld->fd, &ev) == -1) {
		perror("epoll_ctl_mod: listen_sock");
		exit(EXIT_FAILURE);
	}
}