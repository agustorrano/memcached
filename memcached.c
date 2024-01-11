#define _GNU_SOURCE

#include "memcached.h"

#define MAX_EVENTS 100


long nproc;
//struct eventloop_data evld;
//Cache cache;
struct epoll_event ev, ev2;

eventloopData* create_evloop(int epollfd, int text_sock, int bin_sock, int id) {
	eventloopData* info = malloc(sizeof(eventloopData));
	info->bin_sock = bin_sock;
	info->text_sock = text_sock;
	info->epfd = epollfd;
	info->id = id;
	return info;
}

void limit_mem()
{
	struct rlimit* r = malloc(sizeof(struct rlimit));

	if (r = NULL) {
		perror("malloc_rlimit");
		exit(EXIT_FAILURE);
	}

	r->rlim_cur = 100 * 1024 * 1024;
	r->rlim_max = r->rlim_cur;

	if (setrlimit(RLIMIT_DATA, r) < 0) {
		perror("setrlimit");
		exit(EXIT_FAILURE);
	}

	return;
}

void handle_signals()
{
/*Capturar y manejar  SIGPIPE */
}

void init_server(int text_sock, int bin_sock) {
	/* creacion del conjunto epoll */
	int epollfd;

	if ((epollfd = epoll_create1(0)) == -1) {
		perror("epoll_create1");
		exit(EXIT_FAILURE);
	}
	
	/* configuración de sockets para eventos de lectura (EPOLLIN) */

	ev.events = EPOLLIN;
	ev.data.fd = text_sock;
/*text_sock es agregada a la lista de file descriptors*/
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, text_sock, &ev) == -1) {
		perror("epoll_ctl: listen_sock");
		exit(EXIT_FAILURE);
	}

	ev2.events = EPOLLIN;
	ev2.data.fd = bin_sock;
/*bin_sock es agregada a la lista de file descriptors*/
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, bin_sock, &ev2) == -1) {
		perror("epoll_ctl: listen_sock");
		exit(EXIT_FAILURE);
	}

	/* creación de una instancia de eventloopData */
	eventloopData* info = create_evloop(epollfd, text_sock, bin_sock, -1);

	/* configuración de hilos */
	long numofthreads = sysconf(_SC_NPROCESSORS_ONLN);
	pthread_t threads[numofthreads];
	for (int i = 0; i < numofthreads; i++) {
		info->id = i;
		pthread_create(&threads[i], NULL, server, (eventloopData*) info);
	}
}

void server(eventloopData* info) {
	int fds, conn_sock;
	struct epoll_event events[MAX_EVENTS];
	for (;;) { /* la instancia se mantendra esperando nuevos clientes*/
		if ((fds = epoll_wait(info->epfd, events, MAX_EVENTS, -1)) == -1) { 
			perror("epoll_wait");
			exit(EXIT_FAILURE);
		}

		for (int n = 0; n < fds; ++n) {
			if (events[n].data.fd == info->text_sock) { // manejar los clientes del puerto1
				if ((conn_sock = accept(info->text_sock, NULL, NULL)) == -1) {
					quit("accept");
					exit(EXIT_FAILURE);
				}
				ev.events = EPOLLIN;
				ev.data.fd = conn_sock;

				if (epoll_ctl(info->epfd, EPOLL_CTL_ADD, conn_sock, &ev) == -1) {
					perror("epoll_ctl: conn_sock");
					exit(EXIT_FAILURE);
				}
			} 
			else if (events[n].data.fd == info->bin_sock) {
				if ((conn_sock = accept(info->bin_sock, NULL, NULL)) == -1) {
					quit("accept");
					exit(EXIT_FAILURE);
				}
				ev2.events = EPOLLIN;
				ev2.data.fd = conn_sock;

				if (epoll_ctl(info->epfd, EPOLL_CTL_ADD, conn_sock, &ev2) == -1) {
					perror("epoll_ctl: conn_sock");
					exit(EXIT_FAILURE);
				}
			} 
			else  /* atendemos al cliente */
					handle_conn(events[n].data.fd, info->epfd, events[n]);
		}
	}
}


/* 
	enum code command;
	char* buf = malloc(sizeof(char) * 100);
	sprintf(buf, "%s", "GET  V");
	char* toks[3];
	for(int i = 0; i < 3; i++)
		toks[i] = malloc(sizeof(char) * 2048);
	int lens[3];
	command = text_parser(buf, toks, lens);
	printf("checking '%s'\n", code_str(command)); 
  
	*/
