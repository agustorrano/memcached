#include "concqueue.h"

Queue create_queue()
{
  Queue queue = malloc(sizeof(struct _Queue));
  if (queue == NULL) {
    errno = ENOMEM;
		perror("Initializing Structs");
		exit(EXIT_FAILURE);
  }
	queue->first = NULL;
  queue->last = NULL;
	return queue;
}

int empty_queue(Queue queue)
{
	return queue->first == NULL;
}

DNode* search_queue(Queue queue, char* key) {
  for (DNode *p = queue->first; p != NULL; p = p->next)
    if (!strcmp(p->key, key)) return p;
  return NULL;
}

void update_queue(ConcurrentQueue cqueue, char* key, int* flag_enomem) {
  DNode *newNode;
  if (try_malloc(sizeof(struct _DNode), (void*)&newNode) == -1){
    *flag_enomem = 1;
    return;
  }
  newNode->key = strdup(key);
  newNode->next = NULL;
  // pthread_mutex_lock(&cqueue->mutex);
  delete_in_queue(cqueue->queue, key);
  push_queue(cqueue->queue, newNode);
  // pthread_mutex_unlock(&cqueue->mutex);
}

void push_queue(Queue queue, DNode* newNode)
{
  if (empty_queue(queue)) { 
    newNode->prev = NULL;
    queue->first = newNode;
    queue->last = queue->first;
    return;
  }
  newNode->prev = queue->last;
  queue->last->next = newNode;
  queue->last = queue->last->next;
  return;
}

char* pop(Queue queue)
{
  char* ret;
  if (empty_queue(queue)) 
    return NULL; 
  ret = queue->first->key;
	if (!strcmp(queue->first->key, queue->last->key)) {
		queue->first = NULL;
    queue->last = NULL;
    return ret;
  }	
	queue->first = queue->first->next;
  queue->first->prev = NULL;
	return ret;
}

void destroy_queue(Queue queue)
{
  DNode *nodeToDelete;
  while (queue->first != NULL) {
    nodeToDelete = queue->first;
    queue->first = queue->first->next;
    free(nodeToDelete);
  }
	free(queue);
  return;
}

void delete_in_queue(Queue queue, char* key) {
  DNode *found = search_queue(queue, key);
  if (found != NULL) {
    if (queue->last == queue->first){ // tenia un unico elemento
      queue->first = NULL;
      queue->last = NULL;
    }
    else if (queue->last == found) {
      queue->last = found->prev;
      queue->last->next = NULL;
    }
    else if (queue->first == found) {
      queue->first = found->next;
      queue->first->prev = NULL;
    }
    else {
      DNode *previous = found->prev;
      previous->next = found->next;
      DNode *next = found->next;
      next->prev = found->prev;
    }
    free(found->key);
    free(found);
  }
  return;
}

void delete_in_concurrent_queue(ConcurrentQueue concurrentQueue, char* key) {
  pthread_mutex_lock(&concurrentQueue->mutex);
  delete_in_queue(concurrentQueue->queue, key);
  pthread_mutex_unlock(&concurrentQueue->mutex);
  return;
}

void init_concurrent_queue(ConcurrentQueue concurrentQueue)
{
  concurrentQueue->queue = create_queue();
  config_mutex(&concurrentQueue->mutex);
  return;
}

char* pop_concurrent_queue(ConcurrentQueue concurrentQueue)
{
  char* ret;
  pthread_mutex_lock(&concurrentQueue->mutex);
  ret = pop(concurrentQueue->queue);
  pthread_mutex_unlock(&concurrentQueue->mutex);
  return ret;
}

int empty_concurrent_queue(ConcurrentQueue concurrentQueue)
{
	int flag;
	pthread_mutex_lock(&concurrentQueue->mutex);
	flag = empty_queue(concurrentQueue->queue);
	pthread_mutex_unlock(&concurrentQueue->mutex);
	return flag;
}

void destroy_concurrent_queue(ConcurrentQueue concurrentQueue)
{
  destroy_queue(concurrentQueue->queue);
  pthread_mutex_destroy(&concurrentQueue->mutex);
  free(concurrentQueue);
  return;
}

void remove_from_queue(Queue queue, DNode* node) {
  if (queue->last == queue->first){ // tenia un unico elemento
      queue->first = NULL;
      queue->last = NULL;
    }
    else if (queue->last == node) {
      queue->last = node->prev;
      queue->last->next = NULL;
    }
    else if (queue->first == node) {
      queue->first = node->next;
      queue->first->prev = NULL;
    }
    else {
      DNode *previous = node->prev;
      previous->next = node->next;
      DNode *next = node->next;
      next->prev = node->prev;
    }
}
