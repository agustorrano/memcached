#include "memcached.h"

struct epoll_event ev;
eventloopData info;

void limit_mem()
{
	struct rlimit* r = malloc(sizeof(struct rlimit));
	if (r == NULL) {
		perror("malloc_rlimit");
		exit(EXIT_FAILURE);
	}
	r->rlim_cur = MEMORY_LIMIT; /* 1GB por defecto */
	r->rlim_max = r->rlim_cur;
	if (setrlimit(RLIMIT_DATA, r) < 0) {
		perror("setrlimit");
		exit(EXIT_FAILURE);
	}
	free(r);
	return;
}

void sigpipe_handler(int signo) {
	log(1, "Received SIGPIPE");
}

/* manejador de señales para SIGPIPE */
void handle_signals() {
	if (signal(SIGPIPE, sigpipe_handler) == SIG_ERR) {
    perror("handle_signals");
    exit(EXIT_FAILURE);
  }
}

void init_server(int text_sock, int bin_sock) {
	
	/* creacion del conjunto epoll */
	int epollfd;
	if ((epollfd = epoll_create1(0)) == -1) {
		perror("epoll_create1");
		exit(EXIT_FAILURE);
	}
	epoll_ctl_add(epollfd, ev, text_sock, -1, -1);
	epoll_ctl_add(epollfd, ev, bin_sock, -1, -1);

	/* configuración de hilos y estructuras necesarias */
	numofthreads = sysconf(_SC_NPROCESSORS_ONLN);
	pthread_t threads[numofthreads];

	info = create_evloop(epollfd, text_sock, bin_sock);
	
	statsTh = malloc(sizeof(Stats)*numofthreads);
	if (statsTh == NULL) {
		errno = ENOMEM;
		perror("Initializing Structs");
		exit(EXIT_FAILURE);
	}
	//int i = 0;
	//statsTh[i] = create_stats();
	//server(i + (void*)0);
	for (int i = 0; i < numofthreads; i++) {
		statsTh[i] = create_stats();
		if (statsTh[i] == NULL) {
			errno = ENOMEM;
			perror("Initializing Structs");
			exit(EXIT_FAILURE);
		}
		pthread_create(threads + i, NULL, (void *(*)(void *))server, i + (void*)0);
	}
	for (int i = 0; i < numofthreads; i++)
		pthread_join(threads[i], NULL);
	return;
}

void* server(void* arg) {
	int id = arg - (void*)0;
	int fds, conn_sock;
	struct epoll_event events[MAX_EVENTS];
	while (1) { /* la instancia se mantendra esperando nuevos clientes*/
		log(1, "thread waiting");
		if ((fds = epoll_wait(info->epfd, events, MAX_EVENTS, -1)) == -1) { 
			perror("epoll_wait");
			exit(EXIT_FAILURE);
		}
		for (int n = 0; n < fds; ++n) {
			ListeningData ld = events[n].data.ptr;
			if (ld->fd == info->text_sock) { 
				log(3, "accept text-sock");
				if ((conn_sock = accept(info->text_sock, NULL, NULL)) == -1) {
					quit("accept");
					exit(EXIT_FAILURE);
				}
				epoll_ctl_add(info->epfd, ev, conn_sock, TEXT_MODE, id);
			} 
			else if (ld->fd == info->bin_sock) {
				log(3, "accept bin-sock");
				if ((conn_sock = accept(info->bin_sock, NULL, NULL)) == -1) {
					quit("accept");
					exit(EXIT_FAILURE);
				}
				epoll_ctl_add(info->epfd, ev, conn_sock, BIN_MODE, id);
			}
			else  /* atendemos al cliente */ {
				ld->threadId = id;
				handle_conn(ld);
			}
		}
	}
	return NULL;
}

void handle_conn(ListeningData ld) {
	log(3, "start consuming from fd: %d", ld->fd);

	int res;
	/* manejamos al cliente en modo texto */
	if (ld->mode == TEXT_MODE)
		res = text_consume(ld, MAX_BUF_SIZE);

	/* manejamos al cliente en modo binario */
	else 
		res = bin_consume(ld);
	
	log(3, "finished consuming. RES: %d", res);
	if (res == 0)                          /* terminó bien,  volvemos a agregar al cliente*/
		epoll_ctl_mod(info->epfd, ev, ld);
	else {                               /* se corto la conexion */
		close(ld->fd);
		free(ld->client);
		free(ld);
	}
	return;
}
