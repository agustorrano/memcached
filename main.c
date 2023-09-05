#include "hashtable.h"
#include "concqueue.h"
#include "utils.h"
#include "command.h"

Cache cache;
ConcurrentQueue queue;

void* funcion(void *arg) {
  
  int id = arg - (void *)0;

  char *key = malloc(100 * sizeof(char));
  char *val = malloc(100 * sizeof(char));

  sprintf(key, "%d", id);
  if (id == 3) sprintf(val, "%s", "goodbye");
  else sprintf(val, "%s", "hello");

  put(cache, queue, val, key);
  //asm("mfence");

  char* valor = get(cache, queue, key);
  printf("Soy %d, valor found: %s\n", id, valor);

  get_stats(cache);

  free(key);
  free(val);
  return NULL;
}

int main() {
  /* inicializamos estructuras de Datas */
  cache = malloc(sizeof(struct _Cache));
  queue = malloc(sizeof(struct _ConcurrentQueue));
  init_cache(cache, CAPACIDAD_INICIAL_TABLA, (HashFunction)KRHash);
  
  init_concurrent_queue(queue);

  pthread_t clientes[5];
	for (int i = 0; i < 5; i++)
		pthread_create(&clientes[i], NULL, funcion, i + (void *)0);
  for (int i = 0; i < 5; i++)
	  pthread_join(clientes[i], NULL);
   
  map_hashtable(cache->table, (VisitFunction)print_data);
  printf("Capacidad: %d\nNum Elementos: %d\n", cache->table->capacity, cache->table->numElems);

  destroy_cache(cache);
  destroy_concurrent_queue(queue);
  return 0;
}