#define _GNU_SOURCE
#include "utils.h"
#include "sock.h"
#include "memcached.h"
#include "hashtable.h"
Cache cache;
ConcurrentQueue queue;

int main(int argc, char **argv) {
/* creamos dos sockets en modo listen */
	int text_sock, bin_sock;
	__loglevel = 2;

	text_sock = mk_tcp_sock(mc_lport_text);
	if (text_sock < 0)
		quit("mk_tcp_sock.text");

	bin_sock = mk_tcp_sock(mc_lport_bin);
	if (bin_sock < 0)
		quit("mk_tcp_sock.bin");

	/* inicializamos estructuras de datos */
	cache = malloc(sizeof(struct _Cache));
	queue = malloc(sizeof(struct _ConcurrentQueue));
	init_cache(cache, CAPACIDAD_INICIAL_TABLA, (HashFunction)KRHash);
	init_concurrent_queue(queue);

	init_server(text_sock, bin_sock);
    
  destroy_cache(cache);
  destroy_concurrent_queue(queue);
	return;
}