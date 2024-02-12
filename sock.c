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

void do_bindings(int* text_sock, int* bin_sock) {
	in_port_t text_port = mc_lport_text;
	in_port_t bin_port = mc_lport_bin;
	/* chequeamos si tiene privilegios de sudo */
	if (getuid() == 0) {
		text_port = mc_lport_text_p;
		bin_port = mc_lport_bin_p;
	}
	/* creamos dos sockets en modo listen */
	*text_sock = mk_tcp_sock(text_port);
	if (*text_sock < 0) {
		perror("mk_tcp_sock.text");
		exit(EXIT_FAILURE);
	}
	*bin_sock = mk_tcp_sock(bin_port);
	if (*bin_sock < 0) {
		perror("mk_tcp_sock.bin");
		exit(EXIT_FAILURE);
	}

}


int drop_root_privileges() {  // returns 0 on success and -1 on failure
	gid_t gid;
	uid_t uid;

	// no need to "drop" the privileges that you don't have in the first place!
	if (getuid() != 0) {
		return 0;
	}

	// when your program is invoked with sudo, getuid() will return 0 and you
	// won't be able to drop your privileges
	if ((uid = getuid()) == 0) {
		const char *sudo_uid = secure_getenv("SUDO_UID");
		if (sudo_uid == NULL) {
			printf("environment variable `SUDO_UID` not found\n");
			return -1;
		}
		errno = 0;
		uid = (uid_t) strtoll(sudo_uid, NULL, 10);
		if (errno != 0) {
			perror("under-/over-flow in converting `SUDO_UID` to integer");
			return -1;
		}
	}

	// again, in case your program is invoked using sudo
	if ((gid = getgid()) == 0) {
		const char *sudo_gid = secure_getenv("SUDO_GID");
		if (sudo_gid == NULL) {
			printf("environment variable `SUDO_GID` not found\n");
			return -1;
		}
		errno = 0;
		gid = (gid_t) strtoll(sudo_gid, NULL, 10);
		if (errno != 0) {
			perror("under-/over-flow in converting `SUDO_GID` to integer");
			return -1;
		}
	}

	if (setgid(gid) != 0) {
		perror("setgid");
		return -1;
	}
	if (setuid(uid) != 0) {
		perror("setgid");
		return -1;
	}

	// change your directory to somewhere else, just in case if you are in a
	// root-owned one (e.g. /root)
	if (chdir("/") != 0) {
		perror("chdir");
		return -1;
	}

	// check if we successfully dropped the root privileges
	if (setuid(0) == 0 || seteuid(0) == 0) {
		printf("could not drop root privileges!\n");
		return -1;
	}

	return 0;
}
