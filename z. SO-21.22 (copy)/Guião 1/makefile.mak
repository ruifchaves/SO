CC = gcc
CFLAGS = -O2 -Wall

nl: nl.o rl.o

nl.o: nl.c rl.h

rl.o: rl.c rl.h

clean: 
    @rm -f nl *.o