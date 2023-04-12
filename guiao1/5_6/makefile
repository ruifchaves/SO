CC = gcc
CFLAGS = -Wall

person: person.o
	$(CC) $(CFLAGS) -o person person.o

person.o: person.h person.c
	$(CC) $(CFLAGS) -c person.c

clean:
	rm -f person *.o