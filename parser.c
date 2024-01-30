#include "parser.h"

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
      Stats allStats = create_stats(&flag_enomem);
      if (allStats == NULL) res = EOOM;
		  else {
        res = get_stats(statsTh, allStats);
        blen = print_stats(cache, allStats, &buf, &flag_enomem);
        if (blen == -1) res = EOOM;
      }
		  break;
		default: /* EINVALID */
      res = command;
	}
  if (client->mode == TEXT_MODE) {
    if (write_text(res, buf, blen, client->fd) == -1) { return -1; }
  }
  else {
    if (write_bin(res, buf, blen, client->fd) == -1) { return -1; }
  }
  return 0;
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

  if (ntok == 2 && !strcmp(comm, "PUT")) command = PUT;
  else if (ntok == 1 && !strcmp(comm, "DEL")) command = DEL;
  else if (ntok == 1 && !strcmp(comm, "GET")) command = GET;
  else if (ntok == 0 && !strcmp(comm, "STATS")) command = STATS;
  else command = EINVALID;

	return command;
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

int text_consume(ClientData client, char* buf, int size)
{
  if (client->buf != NULL) memcpy(buf, client->buf, client->lenBuf);
  int nread = READ(client->fd, buf + client->lenBuf, size);
  int nlen = nread + client->lenBuf;
  int max_i = 5;
  for (int i = 0; nread == size && i < max_i; i++){
    char* buf2;
    if (try_malloc(sizeof(char)*(size*2), (void*)&buf2) == -1) {
      if (handler(client, EOOM, NULL, NULL) == -1) { return -1 };
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

  /* 15 intentos de lectura (por ahi menos seria mejor) */
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
  if (write(fd, &res, 1) < 0) {
    if (errno = EPIPE){ return -1; }
    perror("Error al escribir en el socket");
    exit(EXIT_FAILURE);
  }
  if (buf != NULL) {
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
int main() {
  int fd = open("mensajes/put_e_123.bin", 0);
  // char buf[2048];
  // int res = text_consume(buf, fd, 0);
  int res = bin_consume(fd);
  return 0;
}

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
