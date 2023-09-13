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
    rc = read(fd, binClient->lkey, 4);
    binClient->cursor = KEY;
    break;
  case KEY:
    rc = read(fd, binClient->key, binClient->lkey);
    binClient->cursor = VALUE_SIZE;
    break;
  case VALUE_SIZE:
    rc = read(fd, binClient->lval, 4);
    binClient->cursor = VALUE;
    break;
  case VALUE:
    rc = read(fd, binClient->val, binClient->lval);
    binClient->cursor = FINISH;
    break;
  default:
    return 0;
    break;
  }
  return -1;
}
