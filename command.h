#ifndef __COMMAND_H__
#define __COMMAND_H__
#include "concqueue.h"
#include "hashtable.h"

void put(Cache cache, ConcurrentQueue queue, char* val, char* key);

void del(Cache cache, ConcurrentQueue queue, char* key);

char* get(Cache cache, ConcurrentQueue queue, char* key);

void get_stats(Cache cache);

#endif