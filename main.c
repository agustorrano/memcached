#include "memcached.h"
#include "cache.h"

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
	cache = malloc(sizeof(struct _Cache));
	if (cache == NULL) {
		errno = ENOMEM;
		perror("Initializing Structs");
		exit(EXIT_FAILURE);
	}
	queue = malloc(sizeof(struct _ConcurrentQueue));
	if (queue == NULL) {
		errno = ENOMEM;
		perror("Initializing Structs");
		exit(EXIT_FAILURE);
	}
	init_cache(cache, queue, TABLE_INIT_CAPACITY, (HashFunction)KRHash);
	init_server(text_sock, bin_sock);
  destroy_cache(cache);
	return 0;
}