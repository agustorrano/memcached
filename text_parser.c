#include "text_parser.h"
#include "bin_parser.h"

int are_printable(char* toks[], unsigned lens[], int ntok) {
  for (int i = 0; i < ntok; i++) {
    char* word = toks[i];
    for (unsigned j = 0; j < lens[i]; j++) {
      if (isprint(word[j]) == 0) return 0;
    }
  }
  return 1;
}

enum code text_parser(char *buf, char *toks[MAX_TOKS], unsigned lens[MAX_TOKS])
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
  if (nread <= 0) { return nread; }
	char* p;
  char* p0 = *buf;
  if ((p = memchr(p0, '\n', nread)) == NULL){ return 0; }
  // encontro un '\n': p apunta a ese byte, desde ahi en adelante, es un pedido valido
  int len = p - p0;
  p++;
  client->lenBuf = nread - len - 1;
  *(p + client->lenBuf) = '\0';
  *buf = p;
  return 1;
}

//int text_consume(ListeningData ld, int size)
int text_consume(CTextData client, int fd, int mode, int threadId, int size)
{
  int nlen;
  if (client->lenBuf == MAX_READ) {
  //  int res = consume_and_discard(ld, &buf, MAX_READ);
  //  if (res <= 0) { return res; }
  //  nlen = client->lenBuf;
  }
  //if (client->lenBuf != 0) {
  //  log(1, "lenBuf: <%d> , buf: <%s>", client->lenBuf, client->buf);
  //}
  else {// si ya leyo en consume and discard, no leo devuelta
    int sizeLeft = 30 - client->lenBuf;
    log(1, "buf: <%s>", client->buf);
    int nread = READ(fd, client->buf + client->lenBuf, sizeLeft);
    if (nread <= 0){ 
      perror("read");
      return nread;
    }
    nlen = nread + client->lenBuf;
  }
 	char *p, *p0 = client->buf;
	while ((p = memchr(p0, '\n', nlen)) != NULL) {
		int len = p - p0;
		*p++ = 0;
		char *toks[2]= {NULL};
		unsigned lens[2] = {0};
    if (len >= size) {
      enum code command = EINVALID;
      log(1, "request too big");
      if (handler(command, NULL, NULL, mode, threadId, fd) == -1) { 
        return -1; 
      }
    }
		else {
      enum code command;
		  command = text_parser(p0, toks, lens);
      if (handler(command, toks, lens, mode, threadId, fd) == -1) { 
        return -1; 
      }
    }
		nlen -= len + 1;
		p0 = p;
	}
  *(p0+nlen+1) = 0;
  client->lenBuf = nlen;
  memcpy(client->buf, p0, nlen + 1);
  return 0;
}

int write_text(enum code res, char* buf, unsigned int blen, int fd) {
  if (res == EOOM) buf = NULL;
  const char* command = code_str(res);
  int commandLen = strlen(command);

  /* verifico que la respuesta sea menor que 2048 */
  if (commandLen + blen + 1 > MAX_BUF_SIZE) {
    if (write(fd, "EBIG\n", 5) < 0) {
      return -1;
    }
  }
  else {
    if (write(fd, command, commandLen) < 0) {
      return -1;
    }

    if (buf != NULL) {
      if (write(fd, " ", 1) < 0) {
        return -1;
      }

      if (write(fd, buf, blen) < 0) {
        return -1;
      }
    }

    if (write(fd, "\n", 1) < 0) {
      return -1;
    }
  }
  return 0;
}