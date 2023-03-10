CC = gcc
CFLAGS = -O2 -Wall

mynl: mynl.o
	$(CC) $(CFLAGS) -o mynl mynl.o

mynl.o: mynl.h mynl.c
	$(CC) $(CFLAGS) -c mynl.c

clean:
	rm -f mynl *.o