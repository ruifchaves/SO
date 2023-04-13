CC = gcc
CFLAGS = -Wall -Wno-unused-variable

gui6: gui6.o
	@$(CC) $(CFLAGS) -o gui6 gui6.o

gui6.o: gui6.c
	@$(CC) $(CFLAGS) -c gui6.c

clean:
	@rm -f gui6 *.o
