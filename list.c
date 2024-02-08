#include "list.h"
#include "log.h"

DNode* create_list() {
  DNode* list = malloc(sizeof(DNode));
  list->next = NULL;
  list->prev = NULL;
  return list;
}

int empty_list(DNode* list) { 
  return list == NULL; 
}

void destroy_list(DNode* list) {
  DNode *nodeToDelete;
  while (list != NULL) {
    nodeToDelete = list;
    list = list->next;
    destroy_data(nodeToDelete->data);
    free(nodeToDelete);
  }
}

DNode* insert_beginning_list(DNode* list, Data data, int* flag_enomem) {
  DNode *newNode;
  if (try_malloc(sizeof(DNode), (void*)&newNode) == -1) {
    *flag_enomem = 1;
    return NULL;
  }
  newNode->prev = list;
  if(list->next) {
    list->next->prev = newNode;
  }
  newNode->next = list->next;
  list->next = newNode;
  newNode->data = data;
  return newNode;
}

DNode* search_list(DNode* list, char* key) {
  for (DNode *node = list->next; node != NULL; node = node->next) {
    Data data = node->data;
    if (compare_data(data->key, key)) 
      return node;
  }
  return NULL;
}

DNode* delete_in_list(DNode* list, char* key) {
  DNode* node = search_list(list, key);
  if (node == NULL)
    return NULL;
  node->prev->next = node->next;
  node->next->prev = node->prev;
  return node;
}