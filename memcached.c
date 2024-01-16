#include "memcached.h"

eventloopData* info;
struct epoll_event ev;

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
	
	epoll_ctl_add(epollfd, text_sock, ev);

	epoll_ctl_add(epollfd, bin_sock, ev);

	/* creación de una instancia de eventloopData */
	info = create_evloop(epollfd, text_sock, bin_sock, -1);

	/* configuración de hilos */
	long numofthreads = sysconf(_SC_NPROCESSORS_ONLN);
	pthread_t threads[numofthreads];
	/*
	for (int i = 0; i < numofthreads; i++) {
		info->id = i;
		pthread_create(threads + i, NULL, (void *(*)(void *))server, NULL);
	}
	for (int i = 0; i < numofthreads; i++)
		pthread_join(threads[i], NULL);
	*/
	server();
	return;
}

void* server() {
	int fds, conn_sock;
	int mode;
	struct epoll_event events[MAX_EVENTS];
	while (1) { /* la instancia se mantendra esperando nuevos clientes*/
	//log(3, "thread waiting");
	if ((fds = epoll_wait(info->epfd, events, MAX_EVENTS, -1)) == -1) { 
			perror("epoll_wait");
			exit(EXIT_FAILURE);
		}
		//log(3, "thread woken up");
		for (int n = 0; n < fds; ++n) {
			Data client;
			if (events[n].data.fd == info->text_sock) { // manejar los clientes del puerto1
				log(3, "accept text-sock");
				if ((conn_sock = accept(info->text_sock, NULL, NULL)) == -1) {
					quit("accept");
					exit(EXIT_FAILURE);
				}
				/*int status = fcntl(conn_sock, F_SETFL, fcntl(conn_sock, F_GETFL, 0) | O_NONBLOCK);
				if (status == -1){
				  perror("calling fcntl");
				  exit(EXIT_FAILURE);
				}*/
				//client = create_data(NULL, NULL, TEXT_MODE);
				mode = TEXT_MODE;
				ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
				ev.data.ptr = client;
				if (epoll_ctl(info->epfd, EPOLL_CTL_ADD, conn_sock, &ev) == -1) {
					perror("epoll_ctl: conn_sock");
					exit(EXIT_FAILURE);
				}
				if (epoll_ctl(info->epfd, EPOLL_CTL_MOD, info->text_sock, &ev) == -1) {
					perror("epoll_ctl: text_sock");
					exit(EXIT_FAILURE);
				}
			} 
			else if (events[n].data.fd == info->bin_sock) {
				log(3, "accept bin-sock");
				if ((conn_sock = accept(info->bin_sock, NULL, NULL)) == -1) {
					quit("accept");
					exit(EXIT_FAILURE);
				}
				/*int status = fcntl(conn_sock, F_SETFL, fcntl(conn_sock, F_GETFL, 0) | O_NONBLOCK);
				if (status == -1){
				  perror("calling fcntl");
				  exit(EXIT_FAILURE);
				}*/
				//client = create_data(NULL, NULL, BIN_MODE);
				mode = BIN_MODE;
				ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
				ev.data.ptr = client;
				if (epoll_ctl(info->epfd, EPOLL_CTL_MOD, info->bin_sock, &ev) == -1) {
					perror("epoll_ctl: bin_sock");
					exit(EXIT_FAILURE);
				}
				if (epoll_ctl(info->epfd, EPOLL_CTL_ADD, conn_sock, &ev) == -1) {
					perror("epoll_ctl: conn_sock");
					exit(EXIT_FAILURE);
				}
			} 
			else  /* atendemos al cliente */ {
				handle_conn(mode, conn_sock);
			}
		}
	}
	return NULL;
}

void handle_conn(int mode, int fd) {
	int res;
	char buf[2048];
	int blen = 0;
	log(3, "start consuming");
	/* manejamos al cliente en modo texto */
	if (mode == TEXT_MODE)
		res = text_consume(buf, fd, blen);
	/* manejamos al cliente en modo binario */
	else res = bin_consume(fd);
	log(3, "finished consuming. Res: %d", res);
	/* Hay que ver si el cliente se desconecta o no.
	Si no lo hace, hay que volver a ponerlo en la epoll para
	que acepte mas mensajes. 
	ev.events = EPOLLIN | EPOLLONESHOT;
	ev.data.ptr = client;
	if (epoll_ctl(info->epfd, EPOLL_CTL_MOD, fd, &ev) == -1) {
		perror("epoll_ctl: conn_sock");
		exit(EXIT_FAILURE);
	}
	*/
}

void text_handle(enum code command, char* toks[MAX_TOKS_T], int lens[MAX_TOKS_T], int fd) {
	switch(command) {
		case PUT:
		put(cache, queue, toks[2], toks[1], TEXT_MODE);
		if (write(fd, "OK\n", 3) < 0) {
			perror("Error al escribir en el socket");
    	exit(EXIT_FAILURE);
		} // habria que ver como manejar errores
		break;
		case GET:
		char* val = get(cache, queue, toks[1]); // creo que si val = NULL, no lo encontró
		char buffer[2048];
  	snprintf(buffer, sizeof(buffer), "OK %s\n", val);
 		if (write(fd, buffer, strlen(buffer)) < 0) {
    	perror("Error al escribir en el socket");
    	exit(EXIT_FAILURE);
  	}
		break;
		case DEL:
		del(cache, queue, toks[1]);
		if (write(fd, "OK\n", 3) < 0) {
			perror("Error al escribir en el socket");
    	exit(EXIT_FAILURE);
		}		
		break;
		case STATS:
		get_stats(cache, fd);
		break;
		default:
  	snprintf(buffer, sizeof(buffer), "%s\n", code_str(command));
 		if (write(fd, buffer, strlen(buffer)) < 0) {
    	perror("Error al escribir en el socket");
    	exit(EXIT_FAILURE);
  	}
	}
}


enum code text_parser(char *buf, char *toks[MAX_TOKS_T], int lens[MAX_TOKS_T])
{
	enum code command;
  char* delim = " ";
  int ntok = 0;

  log(3, "parse(%s)", buf);

	// ! tenemos que cambiar strtok pq no se puede usar con muchos hilos (race condition)
	char* saveptr;
  for (char* token = strtok_r(buf, delim, &saveptr); token != NULL; token = strtok_r(NULL, delim, &saveptr)) {
	if (ntok == MAX_TOKS_T) return command = EINVALID;
    toks[ntok] = token;
    lens[ntok] = strlen(toks[ntok]); // PARA QUE USARIAMOS LOS LENS?
    ntok++;
  }

  if (ntok == 3 && !strcmp(toks[0], "PUT")) command = PUT;
  else if (ntok == 2 && !strcmp(toks[0], "DEL")) command = DEL;
  else if (ntok == 2 && !strcmp(toks[0], "GET")) command = GET;
  else if (ntok == 1 && !strcmp(toks[0], "STATS")) command = STATS;
  else command = EINVALID;

  // log(3, "checking '%s', ntok = %i", code_str(command), ntok);
	printf("checking '%s', ntok = %i\n", code_str(command), ntok);
	return command;
}

int text_consume(char buf[2048], int fd, int blen)
{
	while (1) {
		//int rem = sizeof *buf - blen;
    int rem = 2048 - blen;
    //log(3, "rem %d", rem);
		assert (rem >= 0);
		
		/* Buffer lleno, no hay comandos, matar */
		if (rem == 0)
			return -1;
		int nread = READ(fd, buf + blen, rem);

		//log(3, "Read %i bytes from fd %i", nread, fd);
		if (nread != -1)
      blen += nread;
		char *p, *p0 = buf;
		int nlen = blen;

		/* Para cada \n, procesar, y avanzar punteros */
		while ((p = memchr(p0, '\n', nlen)) != NULL) {
			/* Mensaje completo */
			int len = p - p0;
			*p++ = 0;
			log(3, "full command: <%s>", p0);
			char *toks[3]= {NULL};
			int lens[3] = {0};
			enum code command;
			command = text_parser(p0,toks,lens);
			
			/* 
			  TODO: En esta función tenemos que ejecutar las funciones en cache.c según el comando 
				TODO: text_handle(evd, p0, len, ...);
			*/
      text_handle(command, toks, lens, fd);
			nlen -= len + 1;
			p0 = p;
		}

		/* Si consumimos algo, mover */
		if (p0 != buf) {
			memmove(buf, p0, nlen);
			blen = nlen;
		}
	}
}

int main() {
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
