void my_conc_push(ConcurrentQueue concQueue, char* key, int* flag_enomem)
{
  DNode *newNode;
  if (try_malloc(sizeof(DNode), (void*)&newNode) == -1){
    *flag_enomem = 1;
    return;
  }
  pthread_mutex_lock(&concQueue->mutex);
  Queue queue = concQueue->queue;
  DNode *found = search_queue(queue, key);
  if (found == NULL) {
    newNode->key = strdup(key);
    newNode->next = NULL;
    if (empty_queue(queue)) { 
      newNode->prev = NULL;
      queue->first = newNode;
      queue->last = queue->first;
      pthread_mutex_unlock(&concQueue->mutex);
      return;
    }
    newNode->prev = queue->last;
    queue->last->next = newNode;
  }
  else {
    free(newNode);
    // si la queue tiene ese solo elemento, o bien el elemento ya esta
    // al final de la queue, no es necesario realizar nada
    if(!strcmp(queue->first->key, queue->last->key) 
    || !strcmp(found->key, queue->last->key)) {
      pthread_mutex_unlock(&concQueue->mutex);
      return;
    }
    if(!strcmp(found->key, queue->first->key)) {
      queue->first = queue->first->next;
      queue->first->prev = NULL;
    }
    else {
      DNode *previous = found->prev;
      DNode *next = found->next;
      previous->next = found->next;
      next->prev = found->prev;
    }
    found->next = NULL;
    found->prev = queue->last;
    queue->last->next = found;
  }
  queue->last = queue->last->next;
        pthread_mutex_unlock(&concQueue->mutex);

  return;
}