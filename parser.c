#include "parser.h"

Cache cache;
ConcurrentQueue queue;

// para que necesitariamos el int que devuelve text consume?
int text_consume(char buf[], int fd, int blen, size_t size)
{
	int rem = size - blen;
	assert (rem >= 0);
		
	/* Buffer lleno, no hay comandos, matar */
	if (rem == 0)
		return -1;
	int nread = READ(fd, buf + blen, rem);

	//log(3, "Read %i bytes from fd %i", nread, fd);
	if (nread != -1)
    blen += nread;
	char *p, *p0 = buf;
	int nlen = blen;

	/* Para cada \n, procesar, y avanzar punteros */
	while ((p = memchr(p0, '\n', nlen)) != NULL) {
		int len = p - p0;
		*p++ = 0;
		log(3, "full command: <%s>", p0);
		char *toks[3]= {NULL};
		int lens[3] = {0};
		enum code command;
		command = text_parser(p0,toks,lens);
			
    text_handle(command, toks, lens, fd);
		nlen -= len + 1;
		p0 = p;
	}

	/* Si consumimos algo, mover */
	if (p0 != buf) {
		memmove(buf, p0, nlen);
		blen = nlen;
	}
  return 0;
}


enum code text_parser(char *buf, char *toks[MAX_TOKS_T], int lens[MAX_TOKS_T])
{
	enum code command;
  char* delim = " ";
  int ntok = 0;

  log(3, "parse(%s)", buf);

	char* saveptr;
  for (char* token = strtok_r(buf, delim, &saveptr); token != NULL; token = strtok_r(NULL, delim, &saveptr)) {
	  if (ntok == MAX_TOKS_T) return command = EINVALID;
      toks[ntok] = token;
    lens[ntok] = strlen(toks[ntok]);
    ntok++;
  }

  if (ntok == 3 && !strcmp(toks[0], "PUT")) command = PUT;
  else if (ntok == 2 && !strcmp(toks[0], "DEL")) command = DEL;
  else if (ntok == 2 && !strcmp(toks[0], "GET")) command = GET;
  else if (ntok == 1 && !strcmp(toks[0], "STATS")) command = STATS;
  else command = EINVALID;

	return command;
}

int bin_consume(int fd)
{
  // while (1) {
    /* esto es solo de prueba, hay que hacer muchas cosas */
    char *buf = malloc(10000);
    int blen = 0;
    int rem = 10000;
    int flag = 1;
    
    /* 15 intentos de lectura (por ahi menos seria mejor) */
    for (int i = 0; i < 15 && flag; i++) {
      /* habria que hacer un realloc si el buffer es muy chico */

      int nread = READ(fd, buf + blen, rem);
      log(3, "Read %i bytes from fd %i", nread, fd);

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

    // text_handle(command, toks, lens, fd);
  
  // }
  
  return 0;
}

enum code bin_parser(char *buf, char *toks[], int lens[])
{
  printf("hola\n");
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

void text_handle(enum code command, char* toks[MAX_TOKS_T], int lens[MAX_TOKS_T], int fd) {
	switch(command) {
		case PUT:
		put(cache, queue, toks[2], toks[1], TEXT_MODE);
		if (write(fd, "OK\n", 3) < 0) {
			perror("Error al escribir en el socket");
    	exit(EXIT_FAILURE);
		} // habria que ver como manejar errores
		break;
		case GET:
		char* val = get(cache, queue, toks[1]); 
		char buffer[2048];
    if (val == NULL) 
      snprintf(buffer, sizeof(buffer), "ENOTFOUND\n");
  	else
      snprintf(buffer, sizeof(buffer), "OK %s\n", val);
 		if (write(fd, buffer, strlen(buffer)) < 0) {
    	perror("Error al escribir en el socket");
    	exit(EXIT_FAILURE);
  	}
		break;
		case DEL:
		if(del(cache, queue, toks[1]))
      snprintf(buffer, sizeof(buffer), "OK\n");
    else
      snprintf(buffer, sizeof(buffer), "ENOTFOUND\n");
		if (write(fd, buffer, strlen(buffer)) < 0) {
			perror("Error al escribir en el socket");
    	exit(EXIT_FAILURE);
		}		
		break;
		case STATS:
		get_stats(cache, fd);
		break;
		default:
  	snprintf(buffer, sizeof(buffer), "%s\n", code_str(command));
 		if (write(fd, buffer, strlen(buffer)) < 0) {
    	perror("Error al escribir en el socket");
    	exit(EXIT_FAILURE);
  	}
	}
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