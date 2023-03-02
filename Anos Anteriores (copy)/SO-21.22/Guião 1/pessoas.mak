CC = gcc
CFLAGS = -Wall

pessoas: pessoas.o person.o

person.o: person.h pessoas.c

pessoas.o: person.h pessoas.c

clean:
    rm -f pessoas *.o pessoas