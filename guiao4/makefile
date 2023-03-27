CC = gcc
CFLAGS = -Wall

gui4: gui4.o
	$(CC) $(CFLAGS) -o gui4 gui4.o	

gui4.o: gui4.c
	$(CC) $(CFLAGS) -c gui4.c

clean:
	rm -f gui4 *.o