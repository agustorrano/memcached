#include "memcached.h"

struct epoll_event ev;

void limit_mem()
{
	struct rlimit* r = malloc(sizeof(struct rlimit));
	if (r == NULL) {
		perror("malloc_rlimit");
		exit(EXIT_FAILURE);
	}
	r->rlim_cur = 100 * 1024 * 1024; // 100mb
	r->rlim_max = r->rlim_cur;
	if (setrlimit(RLIMIT_DATA, r) < 0) {
		perror("setrlimit");
		exit(EXIT_FAILURE);
	}
	return;
}


void handle_signals()
{
	log(1, "SIGPIPE");
}


void init_server(int text_sock, int bin_sock) {
	/* creacion del conjunto epoll */
	int epollfd;
	if ((epollfd = epoll_create1(0)) == -1) {
		perror("epoll_create1");
		exit(EXIT_FAILURE);
	}
	epoll_ctl_add(epollfd, ev, text_sock, -1);
	epoll_ctl_add(epollfd, ev, bin_sock, -1);

	/* configuración de hilos */
	long numofthreads = sysconf(_SC_NPROCESSORS_ONLN);
	pthread_t threads[numofthreads];
	eventloopData* infoTh[numofthreads];
	statsTh = malloc(sizeof(Stats)*numofthreads);
	for (int i = 0; i < numofthreads; i++) {
		/* creación de una instancia de eventloopData para cada hilo */
		infoTh[i] = create_evloop(epollfd, text_sock, bin_sock, i, numofthreads);
		pthread_create(threads + i, NULL, (void *(*)(void *))server, (void *)infoTh[i]);
	}
	for (int i = 0; i < numofthreads; i++)
		pthread_join(threads[i], NULL);
	
	return;
}

void* server(eventloopData* infoTh) {
	int fds, conn_sock;
	int mode;
	struct epoll_event events[MAX_EVENTS];
	while (1) { /* la instancia se mantendra esperando nuevos clientes*/
		log(1, "thread waiting");
		if ((fds = epoll_wait(infoTh->epfd, events, MAX_EVENTS, -1)) == -1) { 
			perror("epoll_wait");
			exit(EXIT_FAILURE);
		}
		for (int n = 0; n < fds; ++n) {
			ClientData* client = events[n].data.ptr;
			if (client->fd == infoTh->text_sock) { // manejar los clientes del puerto1
				log(3, "accept text-sock");
				if ((conn_sock = accept(infoTh->text_sock, NULL, NULL)) == -1) {
					quit("accept");
					exit(EXIT_FAILURE);
				}
				epoll_ctl_mod(infoTh->epfd, ev, infoTh->text_sock, -1);
				epoll_ctl_add(infoTh->epfd, ev, conn_sock, TEXT_MODE);
			} 
			else if (client->fd == infoTh->bin_sock) {
				log(3, "accept bin-sock");
				if ((conn_sock = accept(infoTh->bin_sock, NULL, NULL)) == -1) {
					quit("accept");
					exit(EXIT_FAILURE);
				}
				epoll_ctl_mod(infoTh->epfd, ev, infoTh->bin_sock, -1);
				epoll_ctl_add(infoTh->epfd, ev, conn_sock, BIN_MODE);
			}
			else  /* atendemos al cliente */ {
				handle_conn(infoTh, client->mode, client->fd);
			}
		}
	}
	return NULL;
}

void handle_conn(eventloopData* infoTh, int mode, int fd) {
	int res;
	size_t size = 2048;
	char buf[size];
	int blen = 0;
	
	log(3, "start consuming from fd: %d", fd);
	/* manejamos al cliente en modo texto */
	if (mode == TEXT_MODE)
		res = text_consume(infoTh, buf, fd, blen, size);
	/* manejamos al cliente en modo binario */
	else 
		res = bin_consume(infoTh, buf, fd, blen, size);
	log(3, "finished consuming. Res: %d", res);
	
	/* Hay que volver a ponerlo en la epoll para
	que acepte mas mensajes. */
	// creo que aca habria que capturar una señal
	// de que el cliente cortó la comunicación, 
	// y sacarlo de la epoll.
	
	if (res) // res = 1, terminó bien
		epoll_ctl_mod(infoTh->epfd, ev, fd, mode); /* volvemos a agregar al cliente habria q ver el mode*/
	else if (!res) // res = 0, se corto la conexion
		close(fd); // faltaria chequear res = -1, nc como funciona
	return;
}

int main() {
	limit_mem();
	/* creamos dos sockets en modo listen */
	int text_sock, bin_sock;
	__loglevel = 4;

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
	cache = malloc(sizeof(struct _Cache));
	queue = malloc(sizeof(struct _ConcurrentQueue));
	init_cache(cache, CAPACIDAD_INICIAL_TABLA, (HashFunction)KRHash);
	init_concurrent_queue(queue);

	init_server(text_sock, bin_sock);
    
  destroy_cache(cache);
  destroy_concurrent_queue(queue);
	return 0;
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
