#include "utils.h"

long numofthreads;

unsigned KRHash(char *s) {
  unsigned hashval;
  for (hashval = 0; *s != '\0'; ++s) 
    hashval = *s + 31 * hashval;
  return hashval;
}

Data create_data(char* val, char* key, int mode) {
  Data data = malloc(sizeof(struct _Data));
  data ->mode = mode;
  data->key = key;
  data->val = val;
  return data;
}

void destroy_data(Data data) { 
  free(data->key);
  free(data->val);
  free(data); 
  return;
}

Data copy_data(Data data) {
  char *val = malloc(sizeof(char) * (1 + strlen(data->val)));
  char *key = malloc(sizeof(char) * (1 + strlen(data->key)));
  strcpy(val, data->val);
  strcpy(key, data->key);
  int mode = data->mode;
  Data newData = create_data(val, key, mode);
  return newData;
}

int compare_data(char* key1, char* key2) {
  return !strcmp(key1, key2);
}

void print_data(Data data) {
  printf(" Valor: %s, Clave: %s\t", data->val, data->key);
  if (data->mode == 0)
    printf(", Modo: binario\n");
  else 
    printf(", Modo: texto\n");
  return;
}
