CC = gcc
CFLAGS = -Wall -Wno-unused-variable

# GCC flags:
# -Wall: Enable all warning messages
# -Werror: Treat all warnings as errors

gui3: gui3.o
	$(CC) $(CFLAGS) -o gui3 gui3.o

gui3.o: gui3.c
	$(CC) $(CFLAGS) -c gui3.c

clean:
	rm -f gui3 *.o