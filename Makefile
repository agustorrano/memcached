#Banderas para compilar
FLAGS = -g -pthread #-Werror -Wall -Wextra -std=c99
EJECUTABLE = programa

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
  main : main.o list.o hashtable.o utils.o concqueue.o 
	gcc -o $(EJECUTABLE)  main.o list.o hashtable.o utils.o concqueue.o 

main.o : main.c
	gcc -c $(FLAGS) main.c

list.o : list.c list.h
	gcc -c $(FLAGS) list.c

utils.o : utils.c utils.h
	gcc -c $(FLAGS) utils.c

hashtable.o : hashtable.c hashtable.h
	gcc -c $(FLAGS) hashtable.c

concqueue.o : concqueue.c concqueue.h
	gcc -c $(FLAGS) concqueue.c

### Limpieza carpeta ###
CLEAR :
	make clear
	rm -f $(EJECUTABLE)
	@clear

### Limpieza de archivos ###
clear :
	rm -f *.o
