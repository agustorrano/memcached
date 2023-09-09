#include "text_parser.h"

enum code text_parser(char *buf, char *toks[MAX_TOKS], int lens[MAX_TOKS])
{
	enum code command;
  char* delim = " ";
  int ntok = 0;

  log(3, "parse(%s)", buf);

	// tenemos que cambiar strtok pq no se puede usar con muchos hilos (race condition)

  for (char* token = strtok(buf, delim); token != NULL; token = strtok(NULL, delim)) {
	if (ntok == MAX_TOKS) return command = EINVALID;
    toks[ntok] = token;
    lens[ntok] = strlen(toks[ntok]);
    ntok++;
  }
//   /* /* /* Separar tokens */
// 	char *p = buf;
// 	toks[ntok] = p;
// 	ntok++;
// 	while (ntok < MAX_TOKS && (p = strchrnul(p, ' ')) && *p) {
// 		/* Longitud token anterior */
// 		lens[ntok-1] = p - toks[ntok-1];
// 		*p++ = 0;
// 		/* Comienzo nueva token */
// 		if (!strcmp(p, " ")) return command = EINVALID;
// 		toks[ntok] = p;
// 		ntok++;
// 	}
// 	lens[ntok-1] = p - toks[ntok-1];

  if (ntok == 3 && !strcmp(toks[0], "PUT")) command = PUT;
  else if (ntok == 2 && !strcmp(toks[0], "DEL")) command = DEL;
  else if (ntok == 2 && !strcmp(toks[0], "GET")) command = GET;
  else if (ntok == 1 && !strcmp(toks[0], "STATS")) command = STATS;
  else command = EINVALID;

  //log(3, "checking '%s', ntok = %i", code_str(command), ntok);
	printf("checking '%s', ntok = %i\n", code_str(command), ntok);
	return command;
}

int text_consume(struct eventloop_data *evd, char buf[2024], int fd, int blen)
{
	while (1) {
		int rem = sizeof *buf - blen;
		assert (rem >= 0);
		/* Buffer lleno, no hay comandos, matar */
		if (rem == 0)
			return -1;
		int nread = READ(fd, buf + blen, rem);

		log(3, "Read %i bytes from fd %i", nread, fd);
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
			command = text_parser(buf,toks,lens);
			
			// En esta función tenemos que ejecutar las funciones en cache.c según el comando
			// text_handle(evd, p0, len, ...); HACER

			nlen -= len + 1;
			p0 = p;
		}

		/* Si consumimos algo, mover */
		if (p0 != buf) {
			memmove(buf, p0, nlen);
			blen = nlen;
		}
	}
	return 0;
}

void text_handle(enum code command, char* toks[MAX_TOKS], int lens[MAX_TOKS]) {
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


void main() {
	cache = malloc(sizeof(struct _Cache));
	queue = malloc(sizeof(struct _ConcurrentQueue));
	init_cache(cache, CAPACIDAD_INICIAL_TABLA, (HashFunction)KRHash);
	init_concurrent_queue(queue);

	enum code command;
	char* buf = malloc(sizeof(char) * 100);
	sprintf(buf, "%s", "GET  V");
	char* toks[3];
	for(int i = 0; i < 3; i++)
		toks[i] = malloc(sizeof(char) * 2048);
	int lens[3];
	command = text_parser(buf, toks, lens);
	printf("checking '%s'\n", code_str(command));
	return;
}
