#ifndef __LIST_H__
#define __LIST_H__
#include "utils.h"

//! @struct _Node
//! @brief Estructura que representa un nodo que guarda un tipo Data.
//! @var data - Data : dato almacenado.
//! @var next - struct _Node* : puntero al siguiente nodo de la lista.
//!
typedef struct _Node {
  Data data;
  struct _Node *next;
} Node;

//! @typedef 
typedef Node *List;

//! @brief Crea una lista vacía.
//!
//! @return list - List : lista creada.
List create_list();


//! @brief Determina si la lista está vacía.
//!
//! @param[in] list - List.
//! @return val - int : 1 si la lista está vacía, 0 en caso contrario.
int empty_list(List list); 


//! @brief Destruye la lista.
//!
//! @param[in] list - List.
void destroy_list(List list);


//! @brief Inserta un dato al final de la lista
//!
//! @param[in] list - List.
//! @param[in] data - Data: dato a insertar.
//! @param[out] list - List : lista con el nuevo dato insertado.
List insert_final_list(List list, Data data);


//! @brief Inserta un dato al inicio de la lista
//!
//! @param[in] list - List.
//! @param[in] data - Data: dato a insertar.
//! @param[out] list - List : lista con el nuevo dato insertado.
List insert_beginning_list(List list, Data data);


//! @brief Realiza un map en la lista.
//!
//! @param[in] list - List.
//! @param[in] visit - VisitFunction : función aplicada a cada elemento.
void map_list(List lista, VisitFunction visit);


//! @brief Crea una copia de la lista.
//!
//! @param[in] list - List.
//! @return newList - List : copia de la lista original.
List copy_list(List list);


//! @brief Verifica si el dato está en la lista.
//! 
//! Si el dato es encontrado, lo retorna.
//! En caso contrario, retorna NULL.
//!
//! @param[in] list - List.
//! @param[in] key - char* : clave del dato a buscar.
//! @return data - Data : dato encontrado (o NULL).
Data search_list(List list, char* key);


//! @brief Elimina un dato de la lista.
//! 
//! Si el dato no está presente, no hace nada.
//!
//! @param[in] list - List.
//! @param[in] key - char* : clave del dato a buscar.
//! @param[out] flag - int* : bandera que determina si efectivamente
//! se eliminó el dato.
List delete_in_list(List list, char* key);
#endif