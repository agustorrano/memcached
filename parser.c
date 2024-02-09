#include "parser.h"

int handler(enum code command, char** toks, unsigned lens[2], int mode, int threadId, int fd) {
  enum code res;
  char* buf = NULL;
  int blen = 0;
  switch(command) {
	  case PUT:
	    res = put(cache, statsTh[threadId], toks[1], toks[0], mode, lens[1]);
	    buf = NULL;
      blen = 0;
      break;
	  case GET:
	    res = get(cache, statsTh[threadId], mode, toks[0], &buf, &blen);
      //log(1, "val: %s", buf);
	    break;
	  case DEL:
	    res = del(cache, statsTh[threadId], toks[0]);
      buf = NULL;
      blen = 0;
	    break;
	  case STATS:
      Stats allStats = create_stats();
      if (allStats == NULL) res = EOOM;
	    else {
        res = get_stats(statsTh, allStats);
        blen = print_stats(cache, allStats, &buf);
        free(allStats);
        if (blen == -1) res = EOOM;
      }
	    break;
	  default: // EINVALID o EOOM
      res = command;
	}
  int output = 0;
  if (mode == TEXT_MODE) {
    if (write_text(res, buf, blen, fd) == -1) { output = -1; }
  }
  else {
    if (write_bin(res, buf, blen, fd) == -1) { output = -1; }
  }
  if (buf != NULL) free(buf);
  return output;
}

int are_printable(char* toks[], unsigned lens[], int ntok) {
  for (int i = 0; i < ntok; i++) {
    char* word = toks[i];
    for (unsigned j = 0; j < lens[i]; j++) {
      if (isprint(word[j]) == 0) return 0;
    }
  }
  return 1;
}

enum code text_parser(char *buf, char *toks[MAX_TOKS], unsigned lens[MAX_TOKS])
{
	enum code command;
  char* delim = " ";
  int ntok = 0;
  char* saveptr;

  char* comm = strtok_r(buf, delim, &saveptr);
  if (comm == NULL) return command = EINVALID;
  for (char* token = strtok_r(NULL, delim, &saveptr); token != NULL; token = strtok_r(NULL, delim, &saveptr)) {
	  if (ntok == MAX_TOKS)
      return command = EINVALID;
    toks[ntok] = token;
    lens[ntok] = strlen(toks[ntok]);
    ntok++;
  }

  if (ntok == 2 && !strcmp(comm, "PUT")) {
    if (are_printable(toks, lens, ntok)) return command = PUT;
  }
  else if (ntok == 1 && !strcmp(comm, "DEL")) {
    if (are_printable(toks, lens, ntok)) return command = DEL;
  }
  else if (ntok == 1 && !strcmp(comm, "GET")) {
    if (are_printable(toks, lens, ntok)) return command = GET;
  }
  else if (ntok == 0 && !strcmp(comm, "STATS")) return command = STATS;

	return command = EINVALID;
}

int consume_and_discard(ListeningData ld, char** buf, int maxRead){
  CTextData client = (CTextData)ld->client;
  int nread = READ(ld->fd, *buf, maxRead); 
  if (nread <= 0) { return nread; }
	char* p;
  char* p0 = *buf;
  if ((p = memchr(p0, '\n', nread)) == NULL){ return 0; }
  // encontro un '\n': p apunta a ese byte, desde ahi en adelante, es un pedido valido
  int len = p - p0;
  p++;
  client->lenBuf = nread - len - 1;
  *(p + client->lenBuf) = '\0';
  *buf = p;
  return 1;
}

int text_consume(ListeningData ld, int size)
{
  CTextData client = (CTextData)ld->client;
  int nlen;
  char* buf;
  if (try_malloc(sizeof(char)*(MAX_READ), (void*)&buf) == -1) 
    return handler(EOOM, NULL, NULL, ld->mode, ld->threadId, ld->fd); 

  if (client->lenBuf == MAX_READ) {
    int res = consume_and_discard(ld, &buf, MAX_READ);
    if (res <= 0) { return res; }
    nlen = client->lenBuf;
  }
  else {// si ya leyo en consume and discard, no leo devuelta

    if (client->lenBuf > 0) memcpy(buf, client->buf, client->lenBuf);
    int sizeLeft = MAX_READ - client->lenBuf;
    
    int nread = READ(ld->fd, buf + client->lenBuf, sizeLeft);
    if (nread <= 0){ 
      free(buf);
      perror("read");
      return nread;
    }
    nlen = nread + client->lenBuf;
  }
 	char *p, *p0 = buf;
	while ((p = memchr(p0, '\n', nlen)) != NULL) {
		int len = p - p0;
		*p++ = 0;
		char *toks[2]= {NULL};
		unsigned lens[2] = {0};
    if (len >= size){
      enum code command = EINVALID;
      log(1, "request too big");
      if (handler(command, NULL, NULL, ld->mode, ld->threadId, ld->fd) == -1) { 
        free(buf);
        return -1; 
      }
    }
		else {
      enum code command;
		  command = text_parser(p0, toks, lens);
      if (handler(command, toks, lens, ld->mode, ld->threadId, ld->fd) == -1) { 
        free(buf);
        return -1; 
      }
    }
		nlen -= len + 1;
		p0 = p;
	}
  // en p0 queda el resto del pedido (es incompleto, no termina con \n)
  client->buf = p0;
  client->lenBuf = nlen;
  //log(1, "resto: <%s>, longitud: <%d>", client->buf, client->lenBuf);
  return 0;
}

// client-> cursor va a empezar por 0 en todos los casos asi podemos
// manejar los casos en que leemos la mitad de algo
int bin_consume(ListeningData ld) {
  CBinData client = (CBinData)ld->client;
  // void* sizeb;
  int nread, n; // r
  // printf("en bin_consume\n");

  if (client->state == STATE_COMMAND) {
    // printf("en client->state\n");
    // printf("fd: %d\n", ld->fd);
    client->cursor = 0;
    nread = READ(ld->fd, &client->command, 1);
    if (nread < 0) {
      // printf("error read\n");
      return -1; }

    if (!valid_rq(client->command)) {
      client->command = EINVALID;
      return handler(client->command, client->toks, client->lens, ld->mode, ld->threadId, ld->fd);
    }

    // printf("command: %d\n", client->command);

    if (client->command == STATS)
      return handler(client->command, client->toks, client->lens, ld->mode, ld->threadId, ld->fd);

    client->state = STATE_KSIZE;
  }

  if (client->state == STATE_KSIZE) {
    n = 4 - client->cursor;

    // hacemos client->lens + client->cursor por si, por ej,
    // ya habiamos leido 2 bytes, entonces no pisamos lo que
    // teniamos antes
    nread = READ(ld->fd, client->bytes + client->cursor, n);
    if (nread < 0) return -1;
    client->cursor += nread;

    // si client->cursor == 4 quiere decir que pudimos leer
    // la longitud de la clave completamente
    if (client->cursor == 4) {
      client->lens[0] = ntohl(*(unsigned*)client->bytes);
      // printf("ksize: %d\n", client->lens[0]);
      // para no alocar memoria para dos punteros (todavia no
      // sabemos si necesitamos la memoria para el valor), habria
      // que hacer un realloc con la politica de desalojo
      if (try_malloc(sizeof(char*) * 2, (void**)&client->toks) == -1) 
        return handler(EOOM, NULL, NULL, ld->mode, ld->threadId, ld->fd);

      if (try_malloc(client->lens[0], (void**)&client->toks[0]) == -1)
        return handler(EOOM, NULL, NULL, ld->mode, ld->threadId, ld->fd);

      client->cursor = 0;
      client->state = STATE_KEY;
      //printf("command de ksize: %d\n", client->command);
    }
  }

  if (client->state == STATE_KEY) {
    n = client->lens[0] - client->cursor;
    nread = READ(ld->fd, client->toks[0] + client->cursor, client->lens[0] - client->cursor);
    // printf("pudo leer\n");
    if (nread < 0) {
      // printf("error read\n");
      return -1; }
    client->cursor += nread;
    // printf("key: %s\n", client->toks[0]);

    // si el comando es GET o DEL no leemos el valor
    if (client->command == GET || client->command == DEL)
      return handler(client->command, client->toks, client->lens, ld->mode, ld->threadId, ld->fd);

    // si client->cursor == client->lens[0] quiere decir 
    // que pudimos leer la clave completamente
    if (client->cursor == client->lens[0]) {
      client->cursor = 0;
      client->state = STATE_VSIZE;
    }
  }

  if (client->state == STATE_VSIZE) {
    n = 4 - client->cursor;
    nread = READ(ld->fd, client->bytes + client->cursor, n);
    if (nread < 0) return -1;
    client->cursor += nread;

    if (client->cursor == 4) {
      client->lens[1] = ntohl(*(unsigned*)client->bytes);
      // printf("vsize: %d\n", client->lens[1]);
      try_malloc(client->lens[1], (void**)&client->toks[1]);
      if (client->toks[1] == NULL) { return -1; }
      client->cursor = 0;
      client->state = STATE_VALUE;
    }
  }

  if (client->state == STATE_VALUE) {
    n = client->lens[1] - client->cursor;
    nread = READ(ld->fd, client->toks[1] + client->cursor, client->lens[client->lens[0]] - client->cursor);
    if (nread < 0) return -1;
    client->cursor += nread;
    // printf("value: %s\n", client->toks[1]);

    // si client->cursor == client->lens[1] quiere decir 
    // que pudimos leer el valor completamente
    if (client->cursor == client->lens[client->lens[0]]) {
      client->cursor = 0;
    }

    if (handler(client->command, client->toks, client->lens, ld->mode, ld->threadId, ld->fd) == -1){
      return -1;
    }
  }
  
  return 0;
}

int write_text(enum code res, char* buf, int blen, int fd) {
  if (res == EOOM) buf = NULL;
  const char* command = code_str(res);
  int commandLen = strlen(command);

  /* verifico que la respuesta sea menor que 2048 */
  if (commandLen + blen + 1 > MAX_BUF_SIZE) {
    if (write(fd, "EBIG\n", 5) < 0) {
      if (errno = EPIPE){ return -1; }
      perror("Error al escribir en el socket");
      exit(EXIT_FAILURE);
    }
  }
  else {
    if (write(fd, command, commandLen) < 0) {
      if (errno = EPIPE){ return -1; }
      perror("Error al escribir en el socket");
      exit(EXIT_FAILURE);
    }

    if (buf != NULL) {
      if (write(fd, " ", 1) < 0) {
        if (errno = EPIPE){ return -1; }
        perror("Error al escribir en el socket");
        exit(EXIT_FAILURE);
      }

      if (write(fd, buf, blen) < 0) {
        if (errno = EPIPE){ return -1; }
        perror("Error al escribir en el socket");
        exit(EXIT_FAILURE);
      }
    }

    if (write(fd, "\n", 1) < 0) {
      if (errno = EPIPE){ return -1; }
      perror("Error al escribir en el socket");
      exit(EXIT_FAILURE);
    }
  }
  return 0;
}

int write_bin(enum code res, char* buf, int blen, int fd) {
  // printf("en write bin\n");
  // printf("res en write: %d\n", res);
  if (write(fd, &res, 1) < 0) {
    // printf("error uno\n");
    if (errno == EPIPE){ return -1; }
    perror("Error al escribir en el socket");
    exit(EXIT_FAILURE);
  }
  if (buf != NULL) {
    // printf("buf != NULL\n");
    int len = htonl(blen);
    /* primero escribo la longitud de la respuesta */
    if (write(fd, &len, 4) < 0) {
      if (errno = EPIPE){ return -1; }
      perror("Error al escribir en el socket");
      exit(EXIT_FAILURE);
    }
    if (write(fd, buf, len) < 0) {
      if (errno = EPIPE){ return -1; }
      perror("Error al escribir en el socket");
      exit(EXIT_FAILURE);
    }
  }
  return 0;
}