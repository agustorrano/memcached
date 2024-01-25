#include "parser.h"

void handler(ClientData client, enum code command, char* toks[MAX_TOKS], int lens[MAX_TOKS]) {
  enum code res;
  char* buf = NULL;
  int blen = 0;
	switch(command) {
		case PUT:
		  res = put(cache, statsTh[client->threadId], toks[1], toks[0], client->mode);
		  break;
		case GET:
      log(1, "get");
		  res = get(cache, statsTh[client->threadId], client->mode, toks[0], &buf, &blen);
		  break;
		case DEL:
		  res = del(cache, statsTh[client->threadId], toks[0]);
		  break;
		case STATS:
      Stats allStats = create_stats();
		  res = get_stats(statsTh, allStats);
      blen = print_stats(cache, allStats, &buf);
		  break;
		default: /* EINVALID */
      res = command;
  	  //assert(0);
	}
  if (client->mode == TEXT_MODE)
    write_text(res, buf, blen, client->fd);
  else
    write_bin(res, buf, blen, client->fd);
  return;
}

enum code text_parser(char *buf, char *toks[MAX_TOKS], int lens[MAX_TOKS])
{
	enum code command;
  char* delim = " ";
  int ntok = 0;
  char* saveptr;

  log(3, "parse(%s)", buf);
  
  char* comm = strtok_r(buf, delim, &saveptr);

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

// para que necesitariamos el int que devuelve text consume?
int text_consume(ClientData client, char buf[], int blen, int size)
{
	  int rem = size - blen;
	  assert (rem >= 0);
    //log(3, "Rem: %i blen: %i", rem, blen);
  
	  /* Buffer lleno, no hay comandos, matar */
	  if (rem == 0)
	  	return -1;
	  int nread = READ(client->fd, buf + blen, rem);
    if (nread == -1){
      return 0;
    }
	  else
      blen += nread;
	  char *p, *p0 = buf;
	  int nlen = blen;
    int flag = 0;
	  /* Para cada \n, procesar, y avanzar punteros */
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
      /* habria que leer el resto para descartarlo */
      /* creo que seria leer hasta un \n */
    }

	  /* Si consumimos algo, mover */
	  if (p0 != buf) {
	  	memmove(buf, p0, nlen);
	  	blen = nlen;
	  }
  return 1;
}

int bin_consume(ClientData client, char* buf, int blen, int size)
{
  int flag = 1;

  /* 15 intentos de lectura (por ahi menos seria mejor) */
  for (int i = 0; i < 15 && flag; i++) {
    if (blen == size) {
      size *= 2;
      buf = realloc(buf, size);
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

  handler(client, command, toks, lens);
  
  return 0;
}

void write_text(enum code res, char* buf, int blen, int fd) {
  const char* command = code_str(res);
  int commandLen = strlen(command);

  /* verifico que la respuesta sea menor que 2048 */
  if (commandLen + blen + 1 > MAX_BUF_SIZE) {
    if (write(fd, "EBIG\n", 5) < 0) {
      perror("Error al escribir en el socket");
      exit(EXIT_FAILURE);
    }
  }
  else {
    if (write(fd, command, commandLen) < 0) {
      perror("Error al escribir en el socket");
      exit(EXIT_FAILURE);
    }

    if (buf != NULL) {
      log(1, "blen : %d", blen);
      //char* buff = malloc(sizeof(char)*(blen + 2));
      char* buff;
      try_malloc(sizeof(char)*(blen + 2), (void*)&buff);
      int lenn = snprintf(buff, blen + 2, " %s", buf);
      if (write(fd, buff, lenn) < 0) {
        perror("Error al escribir en el socket");
        exit(EXIT_FAILURE);
      }
      free(buff);
    }

    if (write(fd, "\n", 1) < 0) {
      perror("Error al escribir en el socket");
      exit(EXIT_FAILURE);
    }
  }
  return;
}

void write_bin(enum code res, char* buf, int blen, int fd) {
  if (write(fd, &res, 1) < 0) {
    perror("Error al escribir en el socket");
    exit(EXIT_FAILURE);
  }

  if (buf != NULL) {
    int len = htonl(blen); // cambia de formato little endian a big endian
    
    // opcion 1)
    /*
    char* buff = malloc(sizeof(char)*(len+5));
    int len2 = snprintf(buff, len + 4, "%d%s", len, buf);
    if (write(fd, buff, len2) < 0) {
      perror("Error al escribir en el socket");
      exit(EXIT_FAILURE);
    }
    */
    
    // opcion 2)
    /* primero escribo la longitud de la respuesta */
    if (write(fd, &len, 4) < 0) {
      perror("Error al escribir en el socket");
      exit(EXIT_FAILURE);
    }
    if (write(fd, buf, len) < 0) {
      perror("Error al escribir en el socket");
      exit(EXIT_FAILURE);
    }
  }

  return;
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