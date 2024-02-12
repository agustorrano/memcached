#include "memcached.h"

int main()
{
	/* limitamos memoria */
	limit_mem();

	/* manejamos señales (SIGPIPE, SIGINT, SIGTERM)*/
	handle_signals();
	__loglevel = 4;

	/* creamos sockets y realizamos bind() a los puertos correspondientes */
	int text_sock, bin_sock;
	do_bindings(&text_sock, &bin_sock);
	if (drop_root_privileges() == -1)
	{
		exit(EXIT_FAILURE);
	}

	/* inicializamos estructuras de datos */
	ConcurrentQueue queue;
	cache = malloc(sizeof(struct _Cache));
	if (cache == NULL)
	{
		errno = ENOMEM;
		perror("Initializing Structs");
		exit(EXIT_FAILURE);
	}
	queue = malloc(sizeof(struct _ConcurrentQueue));
	if (queue == NULL)
	{
		errno = ENOMEM;
		perror("Initializing Structs");
		exit(EXIT_FAILURE);
	}
	init_cache(cache, queue, TABLE_INIT_CAPACITY, (HashFunction)KRHash);

	/* inicializamos el servidor */
	init_server(text_sock, bin_sock);
	
	return 0;
}