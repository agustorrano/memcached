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
      int flag_enomem = 0;
      Stats allStats = create_stats();
      if (allStats == NULL) res = EOOM;
	    else {
        res = get_stats(statsTh, allStats);
        blen = print_stats(cache, allStats, &buf);
        if (blen == -1) res = EOOM;
      }
	    break;
	  default: // EINVALID o EOOM
      res = command;
	}

  if (mode == TEXT_MODE) {
    if (write_text(res, buf, blen, fd) == -1) { return -1; }
  }
  else {
    // printf("write bin\n");
    // int fdw = open("respuesta.txt", O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (write_bin(res, buf, blen, fd) == -1) { 
      // printf("error\n");
      return -1; }
  }
  return 0;
}

int are_printable(char* toks[], int lens[], int ntok) {
  for (int i = 0; i < ntok; i++) {
    char* word = toks[i];
    for (int j = 0; j < lens[i]; j++) {
      if (isprint(word[j]) == 0) return 0;
    }
  }
  return 1;
}

enum code text_parser(char *buf, char *toks[MAX_TOKS], int lens[MAX_TOKS])
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
	char* p;
  char* p0 = *buf;
  if ((p = memchr(p0, '\n', nread)) == NULL){ return -1; }
  // encontro un '\n': p apunta a ese byte, desde ahi en adelante, es un pedido valido
  int len = p - p0;
  p++;
  client->lenBuf = nread - len - 1;
  *(p + client->lenBuf) = '\0';
  *buf = p;
  return 0;
}

int text_consume(ListeningData ld, int size)
{
  CTextData client = (CTextData)ld->client;
  int nlen;
  char* buf;
  if (try_malloc(sizeof(char)*(MAX_READ), (void*)&buf) == -1) 
    return handler(EOOM, NULL, NULL, ld->mode, ld->threadId, ld->fd); 

  if (client->lenBuf == MAX_READ) {
    if (consume_and_discard(ld, &buf, MAX_READ) == -1) { return 0; }
    nlen = client->lenBuf;
  }
  else {// si ya leyo en consume and discard, no leo devuelta

    if (client->lenBuf > 0) memcpy(buf, client->buf, client->lenBuf);
    int sizeLeft = MAX_READ - client->lenBuf;
    
    int nread = READ(ld->fd, buf + client->lenBuf, sizeLeft);
    if (nread <= 0){ 
      perror("read");
      return -1;
    }
    nlen = nread + client->lenBuf;
  }
 	char *p, *p0 = buf;
  //log(3, "full buffer: <%s>", buf);
	while ((p = memchr(p0, '\n', nlen)) != NULL) {
		int len = p - p0;
		*p++ = 0;
		// log(3, "full command: <%s>", p0);
		char *toks[2]= {NULL};
		int lens[2] = {0};
    if (len >= size){
      enum code command = EINVALID;
      log(1, "request too big");
      if (handler(command, NULL, NULL, ld->mode, ld->threadId, ld->fd) == -1) { return -1; }
    }
		else {
      enum code command;
		  command = text_parser(p0, toks, lens);
      if (handler(command, toks, lens, ld->mode, ld->threadId, ld->fd) == -1) { return -1; }
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
int bin_consume(ListeningData ld, int size) {
  CBinData client = (CBinData)ld->client;
  void* sizeb;
  int nread, n, r;
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



/*

--------------- FUNCIONES MODIFICADAS ---------------

int handler(ClientData client, enum code command, char* toks[MAX_TOKS], int lens[MAX_TOKS]) {
  enum code res;
  char* buf = NULL;
  int blen = 0;

  switch(command) {
	  case PUT:
	    res = put(cache, statsTh[client->threadId], toks[1], toks[0], client->mode, lens[1]);
	    break;
	  case GET:
	    res = get(cache, statsTh[client->threadId], client->mode, toks[0], &buf, &blen);
	    break;
	  case DEL:
	    res = del(cache, statsTh[client->threadId], toks[0]);
	    break;
	  case STATS:
      int flag_enomem = 0;
      Stats allStats = create_stats();
      if (allStats == NULL) res = EOOM;
	    else {
        res = get_stats(statsTh, allStats);
        blen = print_stats(cache, allStats, &buf);
        if (blen == -1) res = EOOM;
      }
	    break;
	  default: // EINVALID
      res = command;
	}

  if (client->mode == TEXT_MODE)
    if (write_text(res, buf, blen, client->fd) == -1) { return -1; }
  
  else
    if (write_bin(res, buf, blen, client->fd) == -1) { return -1; }

  return 0;
}

int text_consume(ClientData client, char* buf, int size)
{
  if (client->buf != NULL) memcpy(buf, client->buf, client->lenBuf);
  int nread = READ(client->fd, buf + client->lenBuf, size);
  int nlen = nread + client->lenBuf;
  int max_i = 5;
  for (int i = 0; nread == size && i < max_i; i++){
    char* buf2;
    if (try_malloc(sizeof(char)*(size*2), (void*)&buf2) == -1) {
      if (handler(client, EOOM, NULL, NULL) == -1) { return -1; }
      return 0; // no retorno error, porque el -1 lo usamos para cuando se cerro la conexion
    } 
    memcpy(buf2, buf, nlen);
    buf = buf2;
    nread = READ(client->fd, buf + nlen, size);
    if (nread == -1){ 
      perror("read");
      return -1;
    }
    if (nread == 0) { // rc = 0, try again ? 
      perror("read");
      log(1, "nread = 0");
      return -1;
    } 
    nlen += nread;
  }
	char *p, *p0 = buf;
  //log(3, "full buffer: <%s>", buf);
	while ((p = memchr(p0, '\n', nlen)) != NULL) {
		int len = p - p0;
		*p++ = 0;
		log(3, "full command: <%s>", p0);
		char *toks[2]= {NULL};
		int lens[2] = {0};
    if (len >= size){
      enum code command = EINVALID;
      log(1, "request too big");
      if (handler(client, command, NULL, NULL) == -1) { return -1; }
    }
		else {
      enum code command;
		  command = text_parser(p0,toks,lens);
      if (handler(client, command, toks, lens) == -1) { return -1; }
    }
		nlen -= len + 1;
		p0 = p;
	}
  // en p0 queda el resto del pedido (es incompleto, no termina con \n)
  buf = p0;
  client->buf = buf;
  client->lenBuf = nlen;
  log(1, "resto: <%s>, longitud: <%d>", client->buf, client->lenBuf);
  return 0;
}

int bin_consume(ClientData client, char* buf, int blen, int size)
{
  int flag = 1;

  // 15 intentos de lectura (por ahi menos seria mejor)
  for (int i = 0; i < 15 && flag; i++) {
    if (blen == size) {
      size *= 2;
      buf = realloc(buf, size); // NO PODEMOS HACER REALLOC (o hay q hacer un try_realloc aunq sea)
    }

    int nread = READ(client->fd, buf + blen, size);
    log(3, "Read %i bytes from fd %i", nread, client->fd);

    // leemos algo
    if (nread > 0) {
      blen += nread;
      flag = 0;
    }
  }

  char *toks[2] = {NULL};
  int lens[2] = {0};
  enum code command;
  command = bin_parser(buf, toks, lens);

  if (handler(client, command, toks, lens) == -1)
    return -1;
  
  return 0;
}

enum code bin_parser(char *buf, char *toks[], int lens[])
{
  enum code command = buf[0];
  printf("checking '%s'\n", code_str(command));
  int idx = 1;

  if (command != PUT && command != GET && command != DEL && command != STATS) command = EINVALID;

  else if (command == STATS);

  else {
    memcpy(lens, buf + idx, 4);
    lens[0] = ntohl(lens[0]); // cambia de formato big endian a little endian
    idx += 4;
    toks[0] = buf + idx;
    idx += lens[0];
    if (command == PUT) {
      memcpy(lens + 1, buf + idx, 4);
      lens[1] = ntohl(lens[1]); // cambia de formato big endian a little endian
      idx += 4;
      toks[1] = buf + idx;
      idx += lens[1];
    }
  }
  return command;
}



int main() {
  int fd = open("mensajes/get_e.bin", O_RDONLY);
  printf("fd: %d\n", fd);
  ConcurrentQueue queue;
  cache = malloc(sizeof(struct _Cache));
  CBinData cli = create_binData();
  ListeningData ld = create_ld(fd, BIN_MODE, 1, (void*)cli);
  queue = malloc(sizeof(struct _ConcurrentQueue));
  init_cache(cache, queue, TABLE_INIT_CAPACITY, (HashFunction)KRHash);
  statsTh = malloc(sizeof(Stats)*3);
  statsTh[1] = create_stats();
  // char buf[2048];
  // int res = text_consume(buf, fd, 0);
  printf("hola\n");
  int res = bin_consume(ld, fd);
  return 0;
}
/*
int main() {
  char* toks[3];
  for (int i = 0; i < 3; i++)
    toks[i] = malloc(sizeof(char)* 1024);
  int lens[3];

   // creamos el archivo 
  // Abrir el archivo en modo de escritura binaria
  FILE *archivo = fopen("mensajes/get_e.bin", "rb");
  // Obtener el tamaño del archivo
  fseek(archivo, 0, SEEK_END);
  long tamañoArchivo = ftell(archivo);
  char *buf = malloc(sizeof(char)*tamañoArchivo);  
  rewind(archivo);
  fread(buf, 1, tamañoArchivo, archivo);
  fclose(archivo);
  // Leer todos los datos en un buffer
  // Cerrar el archivo
  printf("texto en binario: %s", buf);
  enum code command = bin_parser(buf, toks, lens);
  return 0;
}

  // Datos binarios que deseas escribir en el archivo
  unsigned char datos[] = {0x11};
  // Escribir los datos en el archivo
  size_t escritos = fwrite(datos, sizeof(unsigned char), sizeof(datos) / sizeof(unsigned char), archivo);

  int entero = 1;
  fwrite(&entero, sizeof(int), 1, archivo);
  
  unsigned char datos[] = {0x4A};
  size_t escritos = fwrite(datos, sizeof(unsigned char), sizeof(datos) / sizeof(unsigned char), archivo);
  
  int entero = 2;
  fwrite(&entero, sizeof(int), 1, archivo);
  
  unsigned char datos[] = {0x42, 0x00};
  // Escribir los datos en el archivo
  size_t escritos = fwrite(datos, sizeof(unsigned char), sizeof(datos) / sizeof(unsigned char), archivo);
*/



//_________________________
/*
// funcion que utiliza text_consume
void consume_and_discard(ClientData client, char** buf){
  int nread = READ(client->fd, *buf, MAX_BUF_SIZE); //leo 2048 bytes
	char* p;
  char* p0 = *buf;
  while ((p = memchr(p0, '\n', nread)) == NULL){ // si es =NULL, no encontre '\n'
    nread = READ(client->fd, p0, MAX_BUF_SIZE); //leo nuevamente, piso el buffer?
  }
  // encontro un '\n': p apunta a ese byte, desde ahi en adelante, es un pedido valido
  p++;
  p0 = p;
  //*buf = p;
  while ((p = memchr(p0, '\n', nread)) != NULL) {
	  int len = p - p0;
	  *p++ = 0;
	  log(3, "full command: <%s>", p0);
	  char *toks[2]= {NULL};
	  int lens[2] = {0};
	  enum code command;
	  command = text_parser(p0,toks,lens);
    handler(client, command, toks, lens);
	  p0 = p;
	}
}

int text_consume(ClientData client, char buf[], int blen, int size)
{
	int rem = size - blen;
	assert (rem >= 0);
  log(3, "Rem: %i blen: %i", rem, blen);
	if (rem == 0)
		return -1;
	int nread = READ(client->fd, buf + blen, rem);
  int nread = READ(client->fd, buf, size);
  if (nread == -1){
    return 0;
  }
	 else
    blen += nread;
	char *p, *p0 = buf;
	int nlen = blen;
  int flag = 0;
	while ((p = memchr(p0, '\n', nlen)) != NULL) {
    flag = 1;
		int len = p - p0;
		*p++ = 0;
		log(3, "full command: <%s>", p0);
		char *toks[2]= {NULL};
		int lens[2] = {0};
		enum code command;
		command = text_parser(p0,toks,lens);
    handler(client, command, toks, lens);
		nlen -= len + 1;
		p0 = p;
	}
  if (nread == size && !flag) {
    enum code command = EINVALID;
    log(1, "request too big");
    handler(client, command, NULL, NULL);
    // habria que leer el resto para descartarlo, que seria leer hasta un \n 
    // podriamos llamar a una funcion descartar
    char* buf2;
    try_malloc(sizeof(char)*MAX_BUF_SIZE, (void*)&buf2);
    consume_and_discard(client, &buf2);
    // para mi buf2 habria que guardarlo en un buffer en la estructura cliente
  }
	else if (p0 != buf) {
		memmove(buf, p0, nlen);
		blen = nlen;
	}
  return 1;
}
*/
