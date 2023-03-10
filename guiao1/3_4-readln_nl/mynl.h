#include <sys/types.h>
#include <unistd.h>         //chamadas ao sistema
#include <fcntl.h>          //modos de abertura de ficheiro
#include <string.h>         //memset
#include <stdio.h>          //snprintf

#define LINE_SIZE 20

ssize_t readln(int fd, char *line, size_t size);