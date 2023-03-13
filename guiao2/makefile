CC = gcc
CFLAGS = -Wall

gui2: gui2.o
	$(CC) $(CFLAGS) -o gui2 gui2.o

gui2.o: gui2.c
	$(CC) $(CFLAGS) -c gui2.c

clean:
	rm -f gui2 *.o
