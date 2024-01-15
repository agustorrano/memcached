#include "utils.h"

unsigned KRHash(char *s) {
  unsigned hashval;
  for (hashval = 0; *s != '\0'; ++s) 
    hashval = *s + 31 * hashval;
  return hashval;
}

CData create_cdata(char* val, char* key, int mode) {
  CData cdata = malloc(sizeof(struct _CData));
  cdata ->mode = mode;
  cdata->key = key;
  cdata->val = val;
  return cdata;
}

void destroy_cdata(CData cdata) { 
  free(cdata->key);
  free(cdata->val);
  free(cdata); 
  return;
}

CData copy_cdata(CData cdata) {
  char *val = malloc(sizeof(char) * (1 + strlen(cdata->val)));
  char *key = malloc(sizeof(char) * (1 + strlen(cdata->key)));
  strcpy(val, cdata->val);
  strcpy(key, cdata->key);
  int mode = cdata->mode;
  CData newCData = create_cdata(val, key, mode);
  return newCData;
}

int compare_cdata(char* key1, char* key2) {
  return !strcmp(key1, key2);
}

void print_cdata(CData cdata) {
  printf(" Valor: %s, Clave: %s\t", cdata->val, cdata->key);
  if (cdata->mode == 0)
    printf(", Modo: binario\n");
  else 
    printf(", Modo: texto\n");
  return;
}
