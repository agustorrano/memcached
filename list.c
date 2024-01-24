#include "list.h"

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

List insert_final_list(List lista, Data data) {
  //Node *newNode = malloc(sizeof(Node));
  Node *newNode;
  try_malloc(sizeof(Node), (void*)&newNode);
  newNode->data = copy_data(data);
  newNode->next = NULL;
  if (lista == NULL) return newNode;
  List node = lista;
  for (;node->next != NULL;node = node->next); /* ahora 'nodo' apunta al ultimo elemento en la lista */
  node->next = newNode;
  return lista;
}

List insert_beginning_list(List list, Data data) {
  //Node *newNode = malloc(sizeof(Node));
  Node *newNode;
  try_malloc(sizeof(Node), (void*)&newNode);
  newNode->data = copy_data(data);
  newNode->next = list;
  return newNode;
}

void map_list(List list, VisitFunction visit) {
  for (Node *node = list; node != NULL; node = node->next) visit(node->data);
}

List copy_list(List list) {
  List newList = create_list();
  if (empty_list(list)) return newList;
  for (Node *node = list; node != NULL; node = node->next) {
    newList = insert_final_list(newList, node->data);  
  }
  return newList;
}

Data search_list(List list, char* key) {
  for (Node *node = list; node != NULL; node = node->next)
    if (compare_data(node->data->key, key)) return node->data;
  return NULL;
}

List delete_in_list(List list, char* key) {
  if (compare_data(list->data->key, key)) {
    destroy_data(list->data);
    List newList = list->next; /* si eliminamos el primero */
    free(list);
    return newList;
  }
  Node *node;
  for (node = list; node->next != NULL; node = node->next) {
    if (compare_data(node->next->data->key, key)) { /* si eliminamos en el medio */
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