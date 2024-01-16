#ifndef __EPOLL_H__
#define __EPOLL_H__

#include "utils.h"

struct epoll_event ev;

void epoll_ctl_add(int epfd, int fd);

#endif