#include "bin_parser.h"
#include "text_parser.h"

int handler(enum code command, char** toks, unsigned lens[2], int mode, int threadId, int fd) {
  enum code res;
  char* buf = NULL;
  unsigned int blen = 0;
  switch(command) {
	  case PUT:
	    res = put(cache, statsTh[threadId], toks[1], toks[0], mode, lens[1]);
	    buf = NULL;
      blen = 0;
      break;
	  case GET:
	    res = get(cache, statsTh[threadId], mode, toks[0], &buf, &blen);
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

int parse_command(CBinData client, ListeningData ld) {
  int nread;
  client->cursor = 0;
  nread = READ(ld->fd, &client->command, 1);
  if (nread < 0) {
    // printf("error read\n");
    return -1; }

  if (!valid_rq(client->command)) {
    client->command = EINVALID;
    return handler(client->command, NULL, NULL, ld->mode, ld->threadId, ld->fd);
  }

  if (client->command == STATS)
    return handler(client->command, NULL, NULL, ld->mode, ld->threadId, ld->fd);
  client->state = STATE_KSIZE;
}

int parse_ksize(CBinData client, ListeningData ld){
  int n, nread;
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
    client->klen = ntohl(*(unsigned*)client->bytes);
      
    if (try_malloc(client->klen + 1, (void*)&client->key) == -1)
      return handler(EOOM, NULL, NULL, ld->mode, ld->threadId, ld->fd);
      
    client->cursor = 0;
    client->state = STATE_KEY;
  }
  return 0;
}

int parse_key(CBinData client, ListeningData ld){
  int n, nread;
  n = client->klen - client->cursor;
  nread = READ(ld->fd, client->key + client->cursor, n);
  if (nread < 0) {
    // printf("error read\n");
    return -1; }
  client->cursor += nread; 
    
  // si client->cursor == client->lens[0] quiere decir 
  // que pudimos leer la clave completamente
  if (client->cursor == client->klen) {
    client->key[client->klen] = 0;
    // log(1, "key: %s\n", client->key);
    // si el comando es GET o DEL no leemos el valor
    if (client->command == GET || client->command == DEL) {
      char* toks[2] = {NULL};
      int lens[2] = {0};
      toks[0] = client->key;
      lens[0] = client->klen;
      client->state = STATE_COMMAND;
      return handler(client->command, toks, lens, ld->mode, ld->threadId, ld->fd);
    }
    client->cursor = 0;
    client->state = STATE_VSIZE;
  }
  return 0;
}

int parse_vsize(CBinData client, ListeningData ld){
  int n, nread;
  n = 4 - client->cursor;
  nread = READ(ld->fd, client->bytes + client->cursor, n);
  if (nread < 0) return -1;
  client->cursor += nread;

  if (client->cursor == 4) {
    client->vlen = ntohl(*(unsigned*)client->bytes);
    if (try_malloc(client->vlen + 1, (void*)&client->value) == -1) {
      return handler(EOOM, NULL, NULL, ld->mode, ld->threadId, ld->fd);
    }
    client->cursor = 0;
    client->state = STATE_VALUE;
  }
  return 0;
}

int parse_value(CBinData client, ListeningData ld){  
  int n, nread;
  n = client->vlen - client->cursor;
  nread = READ(ld->fd, client->value + client->cursor, n);
  if (nread < 0) return -1;
  client->cursor += nread;
  // si client->cursor == client->lens[1] quiere decir 
  // que pudimos leer el valor completamente
  if (client->cursor == client->vlen) {
    client->value[client->vlen] = 0;
    // log(1, "value: %s\n", client->value);
    client->cursor = 0;
    char* toks[2] = {NULL};
    int lens[2] = {0};
    toks[0] = client->key;
    toks[1] = client->value;
    lens[0] = client->klen;
    lens[1] = client->vlen;
    client->state = STATE_COMMAND;
    return handler(client->command, toks, lens, ld->mode, ld->threadId, ld->fd);
  }
}

// client-> cursor va a empezar por 0 en todos los casos asi podemos
// manejar los casos en que leemos la mitad de algo
int bin_consume(ListeningData ld) {
  CBinData client = (CBinData)ld->client;

  if (client->state == STATE_COMMAND) {
    if (parse_command(client, ld) == -1) return -1;
  }

  if (client->state == STATE_KSIZE) {
    if (parse_ksize(client, ld) == -1) return -1;
  }

  if (client->state == STATE_KEY) {
    if (parse_key(client, ld) == -1) return -1;
  }

  if (client->state == STATE_VSIZE) {
    if (parse_vsize(client, ld) == -1) return -1;
  }

  if (client->state == STATE_VALUE) {
    if (parse_value(client, ld) == -1) return -1;
  }
  
  return 0;
}

int write_bin(enum code res, char* buf, unsigned int blen, int fd) {
  // printf("en write bin\n");
  // log(1, "res en write: %d", res);
  // log(1, "buf: %s", buf);
  if (write(fd, &res, 1) < 0) {
    return -1;
  }
  if (buf != NULL) {
    // printf("buf != NULL\n");
    int len = htonl(blen);
    /* primero escribo la longitud de la respuesta */
    if (write(fd, &len, 4) < 0) {
      return -1;
    }
    if (write(fd, buf, blen) < 0) {
      return -1;
    }
  }
  return 0;
}