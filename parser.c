#include "parser.h"

enum code text_parser(char *buf, char *toks[MAX_TOKS_T], int lens[MAX_TOKS_T])
{
	enum code command;
  char* delim = " ";
  int ntok = 0;

  log(3, "parse(%s)", buf);

	// ! tenemos que cambiar strtok pq no se puede usar con muchos hilos (race condition)
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

  // log(3, "checking '%s', ntok = %i", code_str(command), ntok);
	printf("checking '%s', ntok = %i\n", code_str(command), ntok);
	return command;
}

int text_consume(char buf[2048], int fd, int blen)
{
	while (1) {
    log(3, "sizeof buf: %d", sizeof(*buf));
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
			command = text_parser(p0,toks,lens);
			
			/* 
			  TODO: En esta función tenemos que ejecutar las funciones en cache.c según el comando 
				TODO: text_handle(evd, p0, len, ...);
			*/

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

int bin_consume(int fd)
{
  int ntoks;
  char *buf = malloc(10000);
  int blen = 0;
  char *toks[2] = {NULL};
  int lens[2] = {0};
  while (1) {
    int rem = sizeof *buf - blen;
    assert(rem >= 0);

    /* Buffer lleno, no hay comandos, matar */
    if (rem == 0)
      return -1;
    int nread = READ(fd, buf + blen, rem);

    log(3, "Read %i bytes from fd %i", nread, fd);
    blen += nread;
    char *p, *p0 = buf;
    int nlen = blen;

    ntoks = bin_parser(buf, toks, lens);
  }
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

/*


int main(){
  char* toks[MAX_TOKS];
  for (int i = 0; i < MAX_TOKS; i++)
    toks[i] = malloc(sizeof(char)* 1024);
  int lens[MAX_TOKS];

   // creamos el archivo 
  // Abrir el archivo en modo de escritura binaria
  FILE *archivo = fopen("get_e.bin", "rb");
  // Obtener el tamaño del archivo
  fseek(archivo, 0, SEEK_END);
  long tamañoArchivo = ftell(archivo);
  char *buf = malloc(sizeof(char)*tamañoArchivo);  
  rewind(archivo);
  fread(buf, 1, tamañoArchivo, archivo);
  fclose(archivo);
  // Leer todos los datos en un buffer
  // Cerrar el archivo
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