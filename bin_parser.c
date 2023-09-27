#include "bin_parser.h"

int bin_consume(int fd) {
  int ntoks;
  char* buf = malloc(10000);
  int blen = 0;
  char* toks[2] = {NULL};
  int lens[2] = {0};
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

    ntoks = bin_parser(fd, buf, toks, lens);
  }
  return 0;
}

int bin_parser(int fd, char *buf, char *toks[], int lens[]) {
  enum code code = buf[0];
  int ntoks;
  printf("checking '%s'\n", code_str(code));
  int idx = 1;
  switch (code)
  {
  case PUT:
    ntoks = 2;
    for (int i = 0; i < ntoks; i++) {
      memcpy(lens + i, buf + idx, 4);
      lens[i] = ntohl(lens[i]); // cambia de formato big endian a little endian
      idx += 4;
      toks[i] = buf + idx;
      idx += lens[i];
    }
    break;
  case GET:
    ntoks = 1;
    for (int i = 0; i < ntoks; i++) {
      memcpy(lens + i, buf + idx, 4);
      lens[i] = ntohl(lens[i]); // cambia de formato big endian a little endian
      idx += 4;
      toks[i] = buf + idx;
      idx += lens[i];
    }
    break;
  case DEL:
    ntoks = 1;
    for (int i = 0; i < ntoks; i++) {
      memcpy(lens + i, buf + idx, 4);
      lens[i] = ntohl(lens[i]); // cambia de formato big endian a little endian
      idx += 4;
      toks[i] = buf + idx;
      idx += lens[i];
    }
    break;
  default:
    ntoks = 0;
    break;
  }
  return 0;
}

int main() {
  int fd = open("put_e_123.bin", 0);
  printf("fd: %d\n", fd);
  int r = bin_consume(fd);
  return 0;
}
