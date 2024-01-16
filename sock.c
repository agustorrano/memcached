#include "sock.h"

int mk_tcp_sock(in_port_t port)
{
	int s, rc;
	struct sockaddr_in sa;
	int yes = 1;

	/* Creación socket */
	s = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (s < 0) {
		perror("socket");
		exit(EXIT_FAILURE);
	}
	rc = setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);
	if (rc != 0) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	
	sa.sin_family = AF_INET;
	sa.sin_port = htons(port);
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	
	/* Binding */
	rc = bind(s, (struct sockaddr*) &sa, sizeof sa);
	if (rc < 0) {
		perror("bind");
		exit(EXIT_FAILURE);
	}
	rc = listen(s, 10);
	if (rc < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}
	return s;
}