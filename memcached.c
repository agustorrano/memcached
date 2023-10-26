#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/resource.h>
#include <assert.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>
#include "sock.h"
#include "common.h"
#include "text_parser.h"
#include "bin_parser.h"
#include "memcached.h"
#include "hashtable.h"

#define MAX_EVENTS 100

long nproc;
struct eventloop_data evld;
Cache cache;

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

void* event_handler() {
	int fds, fd, rc;
	struct epoll_event events[MAX_EVENTS], ev;

	for (;;) {
		if ((fds = epoll_wait(evld.epfd, events, MAX_EVENTS, -1)) == -1) {
			perror("epoll_wait");
			exit(EXIT_FAILURE);
		}

		for (int i = 0; i < fds; i++) {
			fd = events[i].data.fd;

			if (fd == evld.text_sock && (events[i].events & EPOLLIN)) {
				char buf[2024];
				rc = text_consume(buf, fd, 0);
			} else if (fd == evld.bin_sock && (events[i].events & EPOLLIN)) {
				rc = bin_consume(fd);
			}
		}
	}
}

void server(int text_sock, int bin_sock)
{
	int epfd;
	struct epoll_event ev;
	pthread_t threads[nproc];

	if ((epfd = epoll_create1(0)) == -1) {
		perror("epoll_create1");
		exit(EXIT_FAILURE);
	}

	evld.epfd = epfd;
	evld.text_sock = text_sock;
	evld.bin_sock = bin_sock;

	for (int i = 0; i < nproc; i++) {
		pthread_create(&threads[i], NULL, event_handler, NULL);
	}

	ev.events = EPOLLIN;

	/* text_sock es agregada a la lista de file descriptors */
	ev.data.fd = text_sock;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, text_sock, &ev) == -1) {
		perror("epoll_ctl");
		exit(EXIT_FAILURE);
	}
	
	/* bin_sock es agregada a la lista de file descriptors */
	ev.data.fd = bin_sock;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, bin_sock, &ev) == -1) {
		perror("epoll_ctl");
		exit(EXIT_FAILURE);
	}

	pthread_join(threads[0], NULL);
	return;
}

int main(int argc, char **argv)
{
	
	int text_sock, bin_sock;

	__loglevel = 3;

	handle_signals();

	/*Función que limita la memoria*/
	limit_mem();

	text_sock = mk_tcp_sock(mc_lport_text);
	if (text_sock < 0)
		quit("mk_tcp_sock.text");

	bin_sock = mk_tcp_sock(mc_lport_bin);
	if (bin_sock < 0)
		quit("mk_tcp_sock.bin");
	
	init_cache(cache, CAPACIDAD_INICIAL_TABLA, (HashFunction)KRHash);

	/*Iniciar el servidor*/
	server(text_sock, bin_sock);

	return 0;
}
