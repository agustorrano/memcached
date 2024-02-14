#ifndef __SOCK_H
#define __SOCK_H

#define _GNU_SOURCE

#include "common.h"
#include "utils.h"

//! @brief Crea un socket, lo bindea al puerto port y se pone a escuchar .
//!
//! @param[in] port - puerto de la conexión.
//! @return socket - file descriptor del socket creado.
int mk_tcp_sock(in_port_t port);

//! @brief Define los puertos de escucha, según si puede
//! bindearse a puertos privilegiados o no tiene permisos.
//! Llama a la función mk_tcp_sock para crear los sockets de texto
//! y binario con los puertos correspondientes.
//!
//! @param[in] port - puerto de la conexión.
//! @param[out] socket - file descriptor del socket creado.
void do_bindings(int *text_sock, int *bin_sock);

//! @brief Chequea que el programa tenga privilegios de superusuario
//! en ese caso, baja los privilegios, accediento a las variables "SUDO_UID"
//! y "SUDO_GID", y modificando el id de usuario y de grupo respectivamente.
//!
//! @return ret - int -1 en caso de error, 0 en otro caso
int drop_root_privileges(void);
#endif
