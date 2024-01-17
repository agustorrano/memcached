#ifndef __EPOLL_H__
#define __EPOLL_H__

#include "utils.h"

void epoll_ctl_add(int epfd, int fd, struct epoll_event ev);

void epoll_ctl_mod(int epfd, int fd, struct epoll_event ev);
#endif