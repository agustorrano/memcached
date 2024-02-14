#include "sock.h"

int mk_tcp_sock(in_port_t port)
{
	int s, rc;
	struct sockaddr_in sa;
	int yes = 1;

	/* Creación socket */
	s = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (s < 0)
	{
		perror("socket");
		exit(EXIT_FAILURE);
	}
	rc = setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);
	if (rc != 0)
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	sa.sin_family = AF_INET;
	sa.sin_port = htons(port);
	sa.sin_addr.s_addr = htonl(INADDR_ANY);

	/* Binding */
	rc = bind(s, (struct sockaddr *)&sa, sizeof sa);
	if (rc < 0)
	{
		perror("bind");
		exit(EXIT_FAILURE);
	}
	rc = listen(s, 10);
	if (rc < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}
	return s;
}

void do_bindings(int *text_sock, int *bin_sock)
{
	in_port_t text_port = mc_lport_text;
	in_port_t bin_port = mc_lport_bin;
	/* chequeamos si tiene privilegios de sudo */
	if (getuid() == 0)
	{
		log(1, "El servidor escuchará de los puertos 888 y 889");
		text_port = mc_lport_text_p;
		bin_port = mc_lport_bin_p;
	}
	else
	{
		log(1, "El servidor escuchará de los puertos 8888 y 8889");
	}
	/* creamos dos sockets en modo listen */
	*text_sock = mk_tcp_sock(text_port);
	if (*text_sock < 0)
	{
		perror("mk_tcp_sock.text");
		exit(EXIT_FAILURE);
	}
	*bin_sock = mk_tcp_sock(bin_port);
	if (*bin_sock < 0)
	{
		perror("mk_tcp_sock.bin");
		exit(EXIT_FAILURE);
	}
}

int drop_root_privileges()
{
	gid_t gid;
	uid_t uid;

	/* si no tiene privilegios, no necesita hacer nada */
	if (getuid() != 0)
	{
		return 0;
	}

	/* lee la identificación de usuario de la variable SUDO_UID */
	if ((uid = getuid()) == 0)
	{
		const char *sudo_uid = secure_getenv("SUDO_UID");
		if (sudo_uid == NULL)
		{
			printf("environment variable `SUDO_UID` not found\n");
			return -1;
		}
		errno = 0;
		uid = (uid_t)strtoll(sudo_uid, NULL, 10);
		if (errno != 0)
		{
			perror("under-/over-flow in converting `SUDO_UID` to integer");
			return -1;
		}
	}

	/* lee la identificación de grupo de la variable SUDO_GID */
	if ((gid = getgid()) == 0)
	{
		const char *sudo_gid = secure_getenv("SUDO_GID");
		if (sudo_gid == NULL)
		{
			printf("environment variable `SUDO_GID` not found\n");
			return -1;
		}
		errno = 0;
		gid = (gid_t)strtoll(sudo_gid, NULL, 10);
		if (errno != 0)
		{
			perror("under-/over-flow in converting `SUDO_GID` to integer");
			return -1;
		}
	}

	/* cambia la identificación de grupo de 0(root) a gid */
	if (setgid(gid) != 0)
	{
		perror("setgid");
		return -1;
	}

	/* cambia la identificación de usuario de 0(root) a uid */
	if (setuid(uid) != 0)
	{
		perror("setgid");
		return -1;
	}

	if (chdir("/") != 0)
	{
		perror("chdir");
		return -1;
	}

	/* intenta conseguir nuevamente los privilegios, para verificar
	que hayan sido bajados de manera permanente */
	if (setuid(0) == 0 || seteuid(0) == 0)
	{
		printf("could not drop root privileges!\n");
		return -1;
	}

	return 0;
}
