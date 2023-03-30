CC = gcc
CFLAGS = -Wall

gui5: gui5.o
	$(CC) $(CFLAGS) -o gui5 gui5.o

gui5.o: gui5.c
	$(CC) $(CFLAGS) -c gui5.c -o gui5.o

clean:
	rm -f gui5 *.o