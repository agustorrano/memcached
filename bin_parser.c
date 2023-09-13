#include "bin_parser.h"

/* supongo que ya tengo todo completo */
int bin_consume(int fd, BinClient binClient) {
  int rc;
  enum cursor cursor = binClient->cursor;
  switch (cursor)
  {
  case COMMAND:
    rc = read(fd, binClient->code, 1); // lee un byte que representa el comando
    binClient->cursor = KEY_SIZE;
    break;
  case KEY_SIZE:
    break;
  case KEY:
    break;
  case VALUE_SIZE:
    break;
  case VALUE:
    break;
  default:
    break;
  }
}