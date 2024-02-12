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

void handler_sigint_sigterm(){
	log(1, "caught sigint");

	/* liberar memoria */
	destroy_cache(cache);
	for (int i = 0; i < numofthreads; i++)
		destroy_stats(statsTh[i]);
	exit(EXIT_SUCCESS);
}

/* manejador de señales */
void handle_signals() {
	/* ignoramos la señal SIGPIPE */
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
    perror("handle_signals");
    exit(EXIT_FAILURE);
  }
	/* terminamos el servidor de manera exitosa */
	if (signal(SIGINT, handler_sigint_sigterm) == SIG_ERR) {
    perror("handle_signals");
    exit(EXIT_FAILURE);
  }
	
	if (signal(SIGTERM, handler_sigint_sigterm) == SIG_ERR) {
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
		//log(1, "thread waiting");
		if ((fds = epoll_wait(info->epfd, events, MAX_EVENTS, -1)) == -1) { 
			perror("epoll_wait");
			exit(EXIT_FAILURE);
		}
		for (int n = 0; n < fds; ++n) {
			ListeningData ld = events[n].data.ptr;
			if (ld->fd == info->text_sock) { 
				//log(3, "accept text-sock");
				if ((conn_sock = accept(info->text_sock, NULL, NULL)) == -1) {
					quit("accept");
					exit(EXIT_FAILURE);
				}
				epoll_ctl_add(info->epfd, ev, conn_sock, TEXT_MODE, id);
			} 
			else if (ld->fd == info->bin_sock) {
				//log(3, "accept bin-sock");
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
	int res;

	/* manejamos al cliente  */
	if (ld->mode == TEXT_MODE){
		res = text_consume(ld);
	}
	else 
		res = bin_consume(ld);
	
	if (res == 0)                          /* terminó bien,  volvemos a agregar al cliente*/
		epoll_ctl_mod(info->epfd, ev, ld);
	else {                               /* se corto la conexion */
		close(ld->fd);
		free(ld->client);
		free(ld);
	}
	return;
}

