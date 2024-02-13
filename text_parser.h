#ifndef __TEXT_PARSER_H
#define __TEXT_PARSER_H

#define MAX_TOKS 2
#include "epoll.h"
#include "command.h"

//! @brief Parsea el buf, que almacena lo consumido del fd.
//!
//! @param[in] buf - char*.  
//! @param[out] toks - char*[] : array donde se guardarán las distintas secciones del pedido.  
//! @param[out] lens - int[] array donde se guardarán las longitudes de las secciones anteriores.  
//! @return comando obtenido.
enum code text_parser(char *buf, char *toks[MAX_TOKS], unsigned lens[MAX_TOKS]);


//! @brief Consume la entrada del fd del cliente, utilizando la macro READ.
//!
//! Llama a parsear, y luego a manejar pedido.
//!
//! @param[in] ld - ListeningData.  
//! @return ret - int : -1 en caso de que se cerró la conexión, 0 si no.
int text_consume(ListeningData ld);


//! @brief Escribe en el socket del cliente la respuesta del pedido.
//!
//! @param[in] res - enum code.  
//! @param[in] buf - char*.  
//! @param[in] blen - int. 
//! @param[in] fd - int : fd del socket. 
//! @return ret - int : -1 en caso de que se cerró la conexión, 0 si no.
int write_text(enum code res, char* buf, unsigned int blen, int fd);

#endif