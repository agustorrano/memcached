#ifndef __SOCK_H
#define __SOCK_H

#include "utils.h"

//! @brief Crea un socket, lo bindea al puerto port y se pone a escuchar .
//!
//! @param[in] port - puerto de la conexión.
//! @param[out] socket - file descriptor del socket creado.
int mk_tcp_sock(in_port_t port);

#endif
