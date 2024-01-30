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
	r->rlim_cur = 512 * 1024 * 1024; // 500mb podriamos ver de cambiarlo cuando se compila
	r->rlim_max = r->rlim_cur;
	if (setrlimit(RLIMIT_DATA, r) < 0) {
		perror("setrlimit");
		exit(EXIT_FAILURE);
	}
	return;
}

/* Manejar SIGPIPE según sea necesario */
void sigpipe_handler(int signo) {
  log(1,"Received SIGPIPE");
}

/* Configurar el manejador de señales para SIGPIPE */
void handle_signals() {
	struct sigaction sa;
  sa.sa_handler = sigpipe_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  if (sigaction(SIGPIPE, &sa, NULL) == -1) {
    perror("sigaction");
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

	/* configuración de hilos */
	numofthreads = sysconf(_SC_NPROCESSORS_ONLN);
	pthread_t threads[numofthreads];
	info = create_evloop(epollfd, text_sock, bin_sock);
	if (try_malloc(sizeof(Stats)*numofthreads, (void*)&statsTh) == -1){
		errno = ENOMEM;
		perror("Initializing Structs");
		exit(EXIT_FAILURE);
	}
	for (int i = 0; i < numofthreads; i++) {
		statsTh[i] = create_stats();
		if (statsTh[i] == NULL) {
			errno = ENOMEM;
			perror("Initializing Structs");
			exit(EXIT_FAILURE);
		}
		pthread_create(threads + i, NULL, (void *(*)(void *))server, i + (void*)0);
	}
	for (int i = 0; i < numofthreads; i++) // esto es necesario?
		pthread_join(threads[i], NULL);
	return;
}

void* server(void* arg) {
	int id = arg - (void*)0;
	int fds, conn_sock;
	int mode;
	struct epoll_event events[MAX_EVENTS];
	while (1) { /* la instancia se mantendra esperando nuevos clientes*/
		log(1, "thread waiting");
		if ((fds = epoll_wait(info->epfd, events, MAX_EVENTS, -1)) == -1) { 
			perror("epoll_wait");
			exit(EXIT_FAILURE);
		}
		for (int n = 0; n < fds; ++n) {
			ListeningData client = events[n].data.ptr;
			if (client->fd == info->text_sock) { 
				log(3, "accept text-sock");
				if ((conn_sock = accept(info->text_sock, NULL, NULL)) == -1) {
					quit("accept");
					exit(EXIT_FAILURE);
				}
				epoll_ctl_add(info->epfd, ev, conn_sock, TEXT_MODE, id);
			} 
			else if (client->fd == info->bin_sock) {
				log(3, "accept bin-sock");
				if ((conn_sock = accept(info->bin_sock, NULL, NULL)) == -1) {
					quit("accept");
					exit(EXIT_FAILURE);
				}
				epoll_ctl_add(info->epfd, ev, conn_sock, BIN_MODE, id);
			}
			else  /* atendemos al cliente */ {
				// client->threadId = id;
				handle_conn(client);
			}
		}
	}
	return NULL;
}

void handle_conn(ListeningData client) {
	int res;
	char buf[MAX_BUF_SIZE];
	int blen = 0;

	log(3, "start consuming from fd: %d", client->fd);

	/* manejamos al cliente en modo texto */
	if (client->mode == TEXT_MODE)
		res = text_consume(client, buf, MAX_BUF_SIZE);

	/* manejamos al cliente en modo binario */
	else 
		res = bin_consume(client, MAX_BUF_SIZE);
	
	log(3, "finished consuming. RES: %d", res);
	if (res == 0) // res == 0, terminó bien
		epoll_ctl_mod(info->epfd, ev, client); /* volvemos a agregar al cliente */
	else // res == -1, se corto la conexion
		close(client->fd);
	return;
}

int main() {
	limit_mem();
	handle_signals();
	__loglevel = 4;
	int text_sock, bin_sock;
	/* creamos dos sockets en modo listen */
	text_sock = mk_tcp_sock(mc_lport_text);
	if (text_sock < 0) {
		perror("mk_tcp_sock.text");
		exit(EXIT_FAILURE);
	}
	bin_sock = mk_tcp_sock(mc_lport_bin);
	if (bin_sock < 0) {
		perror("mk_tcp_sock.bin");
		exit(EXIT_FAILURE);
	}
	/* inicializamos estructuras de datos */
	ConcurrentQueue queue;
	if (try_malloc(sizeof(struct _Cache), (void*)&cache) == -1) {
		errno = ENOMEM;
		perror("Initializing Structs");
		exit(EXIT_FAILURE);
	}
	if (try_malloc(sizeof(struct _ConcurrentQueue), (void*)&queue) == -1) {
		errno = ENOMEM;
		perror("Initializing Structs");
		exit(EXIT_FAILURE);
	}
	init_cache(cache, queue, TABLE_INIT_CAPACITY, (HashFunction)KRHash);
	init_server(text_sock, bin_sock);
  destroy_cache(cache);
	return 0;
}