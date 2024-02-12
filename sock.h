#ifndef __SOCK_H
#define __SOCK_H

#define _GNU_SOURCE

#include "common.h"
#include "utils.h"

//! @brief Crea un socket, lo bindea al puerto port y se pone a escuchar .
//!
//! @param[in] port - puerto de la conexión.
//! @param[out] socket - file descriptor del socket creado.
int mk_tcp_sock(in_port_t port);

void do_bindings(int* text_sock, int* bin_sock);

int drop_root_privileges(void);
#endif
