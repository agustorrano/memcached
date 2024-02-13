#include "list.h"
#include "log.h"

List create_list() { return NULL; }

int empty_list(List list) { return list == NULL; }

void destroy_list(List list)
{
  Node *nodeToDelete;
  while (list != NULL)
  {
    nodeToDelete = list;
    list = list->next;
    destroy_data(nodeToDelete->data);
    free(nodeToDelete);
  }
}

List insert_beginning_list(List list, Data data, int *flag_enomem)
{
  Node *newNode;
  if (try_malloc(sizeof(Node), (void *)&newNode) == -1)
  {
    *flag_enomem = 1;
    return NULL;
  }
  newNode->data = copy_data(data);
  if (newNode->data == NULL)
  {
    *flag_enomem = 1;
    return NULL;
  }
  newNode->next = list;
  return newNode;
}

Data search_list(List list, char *key)
{
  for (Node *node = list; node != NULL; node = node->next)
  {
    if (compare_data(node->data->key, key))
      return node->data;
  }
  return NULL;
}

List delete_in_list(List list, char *key)
{
  if (compare_data(list->data->key, key))
  {
    destroy_data(list->data);
    List newList = list->next;
    free(list);
    return newList;
  }
  Node *node;
  for (node = list; node->next != NULL; node = node->next)
  {
    if (compare_data(node->next->data->key, key))
    {
      Node *nodeToDelete = node->next;
      node->next = nodeToDelete->next;
      destroy_data(nodeToDelete->data);
      free(nodeToDelete);
      return list;
    }
  }
  if (compare_data(node->data->key, key))
  {
    destroy_data(node->data);
    node = NULL;
  }
  return list;
}