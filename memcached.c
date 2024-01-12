#include "memcached.h"

// long nproc;
struct epoll_event ev, ev2;
eventloopData* info;

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

/*
void handle_signals()
{
Capturar y manejar  SIGPIPE 
}
*/

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
	info = create_evloop(epollfd, text_sock, bin_sock, -1);

	/* configuración de hilos */
	long numofthreads = sysconf(_SC_NPROCESSORS_ONLN);
	pthread_t threads[numofthreads];
	for (int i = 0; i < numofthreads; i++) {
		info->id = i;
		pthread_create(&threads[i], NULL, (void*)server, i + (void*)0);
	}
	return;
}

void server(void* arg) {
	int id = arg - (void*)0;
	int fds, conn_sock;
	struct epoll_event events[MAX_EVENTS];
	for (;;) { /* la instancia se mantendra esperando nuevos clientes*/
		printf("hola soy el thread %d\n", id);
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
			else  /* atendemos al cliente */ {
					// handle_conn(events[n].data.fd, info->epfd, events[n]);
			}
		}
	}
	return;
}

void text_handle(enum code command, char* toks[MAX_TOKS_T], int lens[MAX_TOKS_T]) {
	switch(command) {
		case PUT:
		put(cache, queue, toks[2], toks[1]);
		break;
		case GET:
		get(cache, queue, toks[1]);
		break;
		case DEL:
		del(cache, queue, toks[1]);
		break;
		case STATS:
		get_stats(cache);
		break;
		default:
		;
	}
}

int main() {
/* creamos dos sockets en modo listen */
	int text_sock, bin_sock;
	__loglevel = 2;

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
