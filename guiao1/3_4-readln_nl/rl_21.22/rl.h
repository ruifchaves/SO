#include <unistd.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFFER 10

ssize_t readln(int fd, char *line, size_t size);
int readc(int fd, char *c);
