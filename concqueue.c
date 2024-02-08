#include "concqueue.h"
#include "log.h"

Queue create_queue() {
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

int empty_queue(Queue queue) {
	return queue->first == NULL;
}

void destroy_queue(Queue queue) {
  destroy_list(queue->first);
	free(queue);
  return;
}

void delete_in_queue(Queue queue, DNode* node) {
  if (queue->last == queue->first){ // tenia un unico elemento
    queue->first = NULL;
    queue->last = NULL;
    return;
  }

  if (queue->first == node) {
    queue->first = node->next;
    if (node->next)
      node->next->prev = NULL;
  } else {
    if (node->prev)
      node->prev->next = node->next;
  }

  if (queue->last == node) {
    queue->last = node->prev;
    if (node->prev)
      node->prev->next = NULL;
  } else {
    if (node->next) 
      node->next->prev = node->prev;
  }
  
  return;
}

char* pop(Queue queue)
{
  char* ret;
  if (empty_queue(queue)) 
    return NULL; 
  ret = queue->first->data->key;
	if (!strcmp(queue->first->data->key, queue->last->data->key)) {
		queue->first = NULL;
    queue->last = NULL;
    return ret;
  }	
	queue->first = queue->first->next;
  queue->first->prev = NULL;
	return ret;
}

void update_queue(Queue queue, DNode* node) {
  delete_in_queue(queue, node);
  if (empty_queue(queue))
    queue->first = node;
  else
    queue->last->next = node;
  node->prev = queue->last;
  node->next = NULL;
  queue->last = node;
}

void lock_queue(ConcurrentQueue cq) {
  pthread_mutex_lock(&cq->mutex);
}

void unlock_queue(ConcurrentQueue cq) {
  pthread_mutex_unlock(&cq->mutex);
}

void push_queue(ConcurrentQueue cq, Data data, int* flag_enomem) {
  DNode* newNode;
  if (try_malloc(sizeof(DNode), (void*)&newNode) == -1) {
    *flag_enomem = 1;
    return;
  }
  newNode->data = data;
  lock_queue(cq);
  Queue queue = cq->queue;
  if (empty_queue(queue)) 
    queue->first = newNode;
  else
    queue->last->next = newNode;
  newNode->prev = queue->last;
  queue->last = newNode;
  newNode->next = NULL;
  unlock_queue(cq);
  return;
}

void init_concurrent_queue(ConcurrentQueue concurrentQueue) {
  concurrentQueue->queue = create_queue();
  config_mutex(&concurrentQueue->mutex);
  return;
}

int empty_concurrent_queue(ConcurrentQueue concurrentQueue) {
	int flag;
	lock_queue(concurrentQueue);
	flag = empty_queue(concurrentQueue->queue);
	unlock_queue(concurrentQueue);
	return flag;
}

void destroy_concurrent_queue(ConcurrentQueue concurrentQueue) {
  destroy_queue(concurrentQueue->queue);
  pthread_mutex_destroy(&concurrentQueue->mutex);
  free(concurrentQueue);
  return;
}

void delete_in_concurrent_queue(ConcurrentQueue concurrentQueue, DNode* node) {
  lock_queue(concurrentQueue);
  delete_in_queue(concurrentQueue->queue, node);
  unlock_queue(concurrentQueue);
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

void update_concurrent_queue(ConcurrentQueue cq, DNode* node) {
  lock_queue(cq);
  update_queue(cq->queue, node);
  unlock_queue(cq);
}
