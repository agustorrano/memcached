#include "memcached.h"

struct epoll_event ev;
eventloopData info;

void limit_mem()
{
	struct rlimit *r = malloc(sizeof(struct rlimit));
	if (r == NULL)
	{
		perror("malloc_rlimit");
		exit(EXIT_FAILURE);
	}
	r->rlim_cur = MEMORY_BYTES; /* 1GB por defecto */
	r->rlim_max = r->rlim_cur;
	if (setrlimit(RLIMIT_DATA, r) < 0)
	{
		perror("setrlimit");
		exit(EXIT_FAILURE);
	}
	int gigas = MEMORY_GIGAS;
	log(1, "Memoria Limitada a %d GB", gigas);
	free(r);
	return;
}

void handler_sigint_sigterm()
{
	log(1, "Cerrando Servidor");

	/* liberar memoria */
	destroy_cache(cache);
	for (int i = 0; i < numofthreads; i++)
		destroy_stats(statsTh[i]);
	close(info->bin_sock);
	close(info->text_sock);
	exit(EXIT_SUCCESS);
}

/* manejador de señales */
void handle_signals()
{
	/* ignoramos la señal SIGPIPE */
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
	{
		perror("handle_signals");
		exit(EXIT_FAILURE);
	}
	/* terminamos el servidor de manera exitosa */
	if (signal(SIGINT, handler_sigint_sigterm) == SIG_ERR)
	{
		perror("handle_signals");
		exit(EXIT_FAILURE);
	}

	if (signal(SIGTERM, handler_sigint_sigterm) == SIG_ERR)
	{
		perror("handle_signals");
		exit(EXIT_FAILURE);
	}
}

void init_server(int text_sock, int bin_sock)
{
	log(1, "Inicializando Servidor");
	/* creacion del conjunto epoll */
	int epollfd;
	if ((epollfd = epoll_create1(0)) == -1)
	{
		perror("epoll_create1");
		exit(EXIT_FAILURE);
	}
	epoll_ctl_add(epollfd, ev, text_sock, -1, -1);
	epoll_ctl_add(epollfd, ev, bin_sock, -1, -1);

	/* configuración de hilos y estructuras necesarias */
	numofthreads = sysconf(_SC_NPROCESSORS_ONLN);
	pthread_t threads[numofthreads];

	info = create_evloop(epollfd, text_sock, bin_sock);

	statsTh = malloc(sizeof(Stats) * numofthreads);
	if (statsTh == NULL)
	{
		errno = ENOMEM;
		perror("Initializing Structs");
		exit(EXIT_FAILURE);
	}
	for (int i = 0; i < numofthreads; i++)
	{
		statsTh[i] = create_stats();
		if (statsTh[i] == NULL)
		{
			errno = ENOMEM;
			perror("Initializing Structs");
			exit(EXIT_FAILURE);
		}
		pthread_create(threads + i, NULL, (void *(*)(void *))server, i + (void *)0);
	}
	for (int i = 0; i < numofthreads; i++)
		pthread_join(threads[i], NULL);
	return;
}

void *server(void *arg)
{
	log(1, "Thread lanzado");
	int id = arg - (void *)0;
	int fds, conn_sock;
	struct epoll_event events[MAX_EVENTS];
	while (1)
	{ /* la instancia se mantendra esperando nuevos clientes*/
		log(2, "Thread esperando solicitudes");
		if ((fds = epoll_wait(info->epfd, events, MAX_EVENTS, -1)) == -1)
		{
			perror("epoll_wait");
			exit(EXIT_FAILURE);
		}
		for (int n = 0; n < fds; ++n)
		{
			ListeningData ld = events[n].data.ptr;
			if (ld->fd == info->text_sock)
			{
				log(1, "Cliente en modo texto aceptado");
				if ((conn_sock = accept(info->text_sock, NULL, NULL)) == -1)
				{
					quit("accept");
					exit(EXIT_FAILURE);
				}
				epoll_ctl_add(info->epfd, ev, conn_sock, TEXT_MODE, id);
			}
			else if (ld->fd == info->bin_sock)
			{
				log(1, "Cliente en modo binario aceptado");
				if ((conn_sock = accept(info->bin_sock, NULL, NULL)) == -1)
				{
					quit("accept");
					exit(EXIT_FAILURE);
				}
				epoll_ctl_add(info->epfd, ev, conn_sock, BIN_MODE, id);
			}
			else /* atendemos al cliente */
			{
				ld->threadId = id;
				handle_conn(ld);
			}
		}
	}
	return NULL;
}

void handle_conn(ListeningData ld)
{
	int res;
	log(2, "Empezando a consumir del fd: %d", ld->fd);
	/* manejamos al cliente  */
	if (ld->mode == TEXT_MODE)
	{
		res = text_consume(ld);
	}
	else
		res = bin_consume(ld);
	log(2, "Terminó de consumir del fd: %d", ld->fd);
	if (res == 0 || res == 1) /* terminó bien,  volvemos a agregar al cliente*/
		epoll_ctl_mod(info->epfd, ev, ld);
	else
	{ /* se corto la conexion */
		close(ld->fd);
		free(ld->client);
		free(ld);
	}
	return;
}
