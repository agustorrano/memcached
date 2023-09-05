#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/epoll.h>

#define MAX_EVENTS 100
/*
 * Para probar, usar netcat.
 *			Compilar servidor en una terminal, y en otras hacer:
 *
 *      $ nc localhost 4040
 *      > NUEVO
 *      0
 *      > NUEVO
 *      1
 *      > CHAU
 *
 *			(Una terminal por conexion)
 */

void quit(char *s)
{
	perror(s);
	abort();
}

int U = 0;

int fd_readline(int fd, char *buf)
{
	int rc;
	int i = 0;

	/*
	 * Leemos de a un caracter (no muy eficiente...) hasta
	 * completar una línea.
	 */
	while ((rc = read(fd, buf + i, 1)) > 0) {
		if (buf[i] == '\n')
			break;
		i++;
	}

	if (rc < 0)
		return rc;

	buf[i] = 0;
	return i;
}

void handle_conn(int csock, int epfd,struct epoll_event event)
{
	char buf[200];
	int rc;

	/* Atendemos pedidos, uno por linea */
	rc = fd_readline(csock, buf);
	if (rc < 0)
		quit("read... raro");

	if (rc == 0) {
		/* linea vacia, se cerró la conexión */
		close(csock);
		return;
	}

	if (!strcmp(buf, "NUEVO")) {
		char reply[20];
		sprintf(reply, "%d\n", U);
		write(csock, reply, strlen(reply));
		U++;
	} 
	else if (!strcmp(buf, "CHAU")) {
		epoll_ctl(epfd, EPOLL_CTL_DEL, event.data.fd, NULL);
		close(event.data.fd);
		close(csock);
		return;
	}
}

void wait_for_clients(int lsock) {

	struct epoll_event ev, events[MAX_EVENTS];
	ev.events = EPOLLIN;
	ev.data.fd = lsock;

	int epollfd, fds, conn_sock;

	if ((epollfd = epoll_create1(0)) == -1) {
		perror("epoll_create1");
		exit(EXIT_FAILURE);
	}
	
/*lsock es agregada a la lista de file descriptors*/
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, lsock, &ev) == -1) {
		perror("epoll_ctl: listen_sock");
		exit(EXIT_FAILURE);
	}

	for (;;) { /* la instancia se mantendra esperando nuevos clientes*/
		if ((fds = epoll_wait(epollfd, events, MAX_EVENTS, -1)) == -1) { 
			perror("epoll_wait");
			exit(EXIT_FAILURE);
		}

		for (int n = 0; n < fds; ++n) {
			if (events[n].data.fd == lsock) {
				if ((conn_sock = accept(lsock, NULL, NULL)) == -1) {
					quit("accept");
					exit(EXIT_FAILURE);
				}
				ev.events = EPOLLIN;
				ev.data.fd = conn_sock;

				if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock, &ev) == -1) {
					perror("epoll_ctl: conn_sock");
					exit(EXIT_FAILURE);
				}
			} else  /* atendemos al cliente */
					handle_conn(events[n].data.fd, epollfd, events[n]);
		}
	}
}

/* Crea un socket de escucha en puerto 4040 TCP */
int mk_lsock()
{
	struct sockaddr_in sa;
	int lsock;
	int rc;
	int yes = 1;

	/* Crear socket */
	lsock = socket(AF_INET, SOCK_STREAM, 0);
	if (lsock < 0)
		quit("socket");

	/* Setear opción reuseaddr... normalmente no es necesario */
	if (setsockopt(lsock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == 1)
		quit("setsockopt");

	sa.sin_family = AF_INET;
	sa.sin_port = htons(4040);
	sa.sin_addr.s_addr = htonl(INADDR_ANY);

	/* Bindear al puerto 4040 TCP, en todas las direcciones disponibles */
	rc = bind(lsock, (struct sockaddr *)&sa, sizeof sa);
	if (rc < 0)
		quit("bind");

	/* Setear en modo escucha */
	rc = listen(lsock, 10);
	if (rc < 0)
		quit("listen");

	return lsock;
}

int main()
{
	int lsock;
	lsock = mk_lsock();
	wait_for_clients(lsock);
}
