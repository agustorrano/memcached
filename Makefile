#Banderas para compilar
FLAGS = -g -pthread #-Werror -Wall -Wextra -std=c99
EJECUTABLE = programa

ifndef MEMORY_LIMIT
MEMORY_LIMIT = 1
endif

MEMORY_LIMIT_BYTES = $(shell echo "$$(( $(MEMORY_LIMIT) * 1024 * 1024 ))")

FLAGS += -DMEMORY_LIMIT=$(MEMORY_LIMIT_BYTES)

#-Wall y -Wextra: activan todos las advertencias
#-Werror: convierte las advertencias en errores
#-std=c99: usa el estándar C99
#-g: genera información para el debugging

#-c: Compila el codigo sin linkear, se crea un archivo objeto.
#-o: cambia el nombre por defecto del archivo generado por uno elegido.

### compilado ###
preparado :
	make main
	make clear

### Programa principal ###
  main : list.o hashtable.o utils.o concqueue.o sock.o parser.o command.o common.o log.o memcached.o epoll.o 
	gcc -o $(EJECUTABLE) list.o hashtable.o utils.o concqueue.o sock.o parser.o command.o common.o log.o memcached.o epoll.o

#main.o : main.c
#	gcc -c $(FLAGS) main.c

list.o : list.c list.h
	gcc -c $(FLAGS) list.c

utils.o : utils.c utils.h
	gcc -c $(FLAGS) utils.c

hashtable.o : hashtable.c hashtable.h
	gcc -c $(FLAGS) hashtable.c

concqueue.o : concqueue.c concqueue.h
	gcc -c $(FLAGS) concqueue.c

sock.o : sock.c sock.h 
	gcc -c $(FLAGS) sock.c

parser.o : parser.c parser.h 
	gcc -c $(FLAGS) parser.c

command.o : command.c command.h 
	gcc -c $(FLAGS) command.c

common.o : common.c common.h 
	gcc -c $(FLAGS) common.c

log.o : log.c log.h 
	gcc -c $(FLAGS) log.c

memcached.o : memcached.c memcached.h 
	gcc -c $(FLAGS) memcached.c

epoll.o : epoll.c epoll.h
	gcc -c $(FLAGS) epoll.c
### Limpieza carpeta ###
CLEAR :
	make clear
	rm -f $(EJECUTABLE)
	@clear

### Limpieza de archivos ###
clear :
	rm -f *.o
