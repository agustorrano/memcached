#include "list.h"
#include "log.h"
List create_list() {return NULL;}

int empty_list(List list) { return list == NULL; }

void destroy_list(List list) {
  Node *nodeToDelete;
  while (list != NULL) {
    nodeToDelete = list;
    list = list->next;
    destroy_data(nodeToDelete->data);
    free(nodeToDelete);
  }
}

/*
List insert_final_list(List lista, Data data, int* flag_enomem) {
  Node *newNode;
  if (try_malloc(sizeof(Node), (void*)&newNode) == -1) {
    *flag_enomem = 1;
    return NULL;
  }
  newNode->data = copy_data(data);
  if (newNode->data ==  NULL) {
    *flag_enomem = 1;
    return NULL;
  }
  newNode->next = NULL;
  if (lista == NULL) return newNode;
  List node = lista;
  for (;node->next != NULL;node = node->next);
  node->next = newNode;
  return lista;
}
*/

List insert_beginning_list(List list, Data data, int* flag_enomem) {
  Node *newNode;
  if (try_malloc(sizeof(Node), (void*)&newNode) == -1) {
    *flag_enomem = 1;
    return NULL;
  }
  newNode->data = copy_data(data);
  if (newNode->data == NULL) {
    *flag_enomem = 1;
    return NULL;
  }
  newNode->next = list;
  return newNode;
}

void map_list(List list, VisitFunction visit) {
  for (Node *node = list; node != NULL; node = node->next) visit(node->data);
}

/*
List copy_list(List list) {
  List newList = create_list();
  if (empty_list(list)) return newList;
  for (Node *node = list; node != NULL; node = node->next) {
    newList = insert_final_list(newList, node->data);  
  }
  return newList;
}
*/

Data search_list(List list, char* key) {
  for (Node *node = list; node != NULL; node = node->next) {
    if (compare_data(node->data->key, key)) return node->data;
  }
  return NULL;
}

List delete_in_list(List list, char* key) {
  if (compare_data(list->data->key, key)) {
    destroy_data(list->data);
    List newList = list->next;
    free(list);
    return newList;
  }
  Node *node;
  for (node = list; node->next != NULL; node = node->next) {
    if (compare_data(node->next->data->key, key)) { 
      Node *nodeToDelete= node->next;
      node->next = nodeToDelete->next;
      destroy_data(nodeToDelete->data);
      free(nodeToDelete);
      return list;
    }
  }
  if (compare_data(node->data->key, key)) {
    destroy_data(node->data);
    node = NULL;
  }
  return list;
}