#include "bin_parser.h"
// #include "command.h"
// #include "memcached.h"

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