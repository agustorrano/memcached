#ifndef __LIST_H__
#define __LIST_H__
#include "utils.h"

//! @struct _DNode
//! @brief Estructura que representa nodo de una lista doblemente enlazada.
//! @var key - char * : clave.
//! @var next - _DNode* : puntero al siguiente nodo de la lista.
//! @var prev - _DNode* : puntero al nodo anterior de la lista.
typedef struct _DNode {
  Data data;
  struct _DNode* next;
  struct _DNode* prev;
} DNode;

//! @brief Crea una lista vacía.
//!
//! @return list - List : lista creada.
DNode* create_list();


//! @brief Determina si la lista está vacía.
//!
//! @param[in] list - List.
//! @return val - int : 1 si está vacía, 0 en caso contrario.
int empty_list(DNode* list); 


//! @brief Destruye la lista.
//!
//! @param[in] list - List.
void destroy_list(DNode* list);


//! @brief Inserta un dato al inicio de la lista
//!
//! @param[in] list - List.
//! @param[in] data - Data: dato a insertar.
//! @param[out] flag_enomem - int* : bandera para informar que no se pudo allocar memoria.
//! @return list - List : lista con el nuevo dato insertado.
DNode* insert_beginning_list(DNode* list, Data data, int* flag_enomem);


//! @brief Verifica si el dato está en la lista.
//! 
//! Si el dato es encontrado, lo retorna.
//! En caso contrario, retorna NULL.
//!
//! @param[in] list - List.
//! @param[in] key - char* : clave del dato a buscar.
//! @return data - Data : dato encontrado (o NULL).
DNode* search_list(DNode* list, char* key);


//! @brief Elimina un dato de la lista.
//! 
//! Si el dato no está presente, no hace nada.
//!
//! @param[in] list - List.
//! @param[in] key - char* : clave del dato a buscar.
//! @return list - List : lista con el dato eliminado (o la lista inicial).
DNode* delete_in_list(DNode* list, char* key);

#endif