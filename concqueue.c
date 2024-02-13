#include "concqueue.h"

Queue create_queue()
{
  Queue queue = malloc(sizeof(struct _Queue));
  if (queue == NULL)
  {
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

DNode* search_queue(Queue queue, char *key)
{
  for (DNode *p = queue->first; p != NULL; p = p->next)
    if (!strcmp(p->key, key))
      return p;
  return NULL;
}

void push_queue(Queue queue, DNode *newNode)
{
  if (empty_queue(queue))
  {
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

char *pop(Queue queue)
{
  char *ret;
  if (empty_queue(queue))
    return NULL;
  ret = queue->first->key;
  if (!strcmp(queue->first->key, queue->last->key))
  {
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
  while (queue->first != NULL)
  {
    nodeToDelete = queue->first;
    queue->first = queue->first->next;
    free(nodeToDelete);
  }
  free(queue);
  return;
}

void delete_in_queue(Queue queue, char *key)
{
  DNode *found = search_queue(queue, key);
  if (found != NULL)
  {
    if (queue->last == queue->first)
    { // tenia un unico elemento
      queue->first = NULL;
      queue->last = NULL;
    }
    else if (queue->last == found)
    {
      queue->last = found->prev;
      queue->last->next = NULL;
    }
    else if (queue->first == found)
    {
      queue->first = found->next;
      queue->first->prev = NULL;
    }
    else
    {
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

void init_concurrent_queue(ConcurrentQueue concurrentQueue)
{
  concurrentQueue->queue = create_queue();
  config_recursive_mutex(&concurrentQueue->mutex);
  return;
}

void destroy_concurrent_queue(ConcurrentQueue concurrentQueue)
{
  destroy_queue(concurrentQueue->queue);
  pthread_mutex_destroy(&concurrentQueue->mutex);
  free(concurrentQueue);
  return;
}

void delete_in_concurrent_queue(ConcurrentQueue concurrentQueue, char *key)
{
  pthread_mutex_lock(&concurrentQueue->mutex);
  delete_in_queue(concurrentQueue->queue, key);
  pthread_mutex_unlock(&concurrentQueue->mutex);
  return;
}


void update_queue(ConcurrentQueue cqueue, char *key, int *flag_enomem)
{
  pthread_mutex_lock(&cqueue->mutex);
  DNode *found = search_queue(cqueue->queue, key);
  if (found == NULL)
  {
    if (try_malloc(sizeof(struct _DNode), (void *)&found) == -1)
    {
      *flag_enomem = 1;
      return;
    }
    found->key = strdup(key);
  }
  else
  {
    remove_from_queue(cqueue->queue, found, 0);
  }
  found->next = NULL;
  push_queue(cqueue->queue, found);
  pthread_mutex_unlock(&cqueue->mutex);
}

/* devuelve el siguiente al nodo que sacó */
DNode *remove_from_queue(Queue queue, DNode *node, int flag)
{
  DNode *ret;
  if (queue->last == queue->first)
  { // tenia un unico elemento
    queue->first = NULL;
    queue->last = NULL;
    ret = NULL;
  }
  else if (queue->last == node)
  {
    queue->last = node->prev;
    queue->last->next = NULL;
    ret = NULL;
  }
  else if (queue->first == node)
  {
    queue->first = node->next;
    queue->first->prev = NULL;
    ret = queue->first;
  }
  else
  {
    DNode *previous = node->prev;
    previous->next = node->next;
    DNode *next = node->next;
    next->prev = node->prev;
    ret = next;
  }
  if (flag == 1)
  {
    free(node->key);
    free(node);
  }
  return ret;
}
