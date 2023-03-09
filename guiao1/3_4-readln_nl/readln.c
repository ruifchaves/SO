#include <sys/types.h>
#include <unistd.h>         //chamadas ao sistema
#include <fcntl.h>          //modos de abertura de ficheiro

#define BUFFER_SIZE 2048
char buffer[BUFFER_SIZE];


ssize_t readln(int fd, char *line, size_t size){
    size_t bytes_read;
    
    int i = 0;
    while(i < size && readc(fd, &line[i]) > 0){
        i++;
        if (((char*) line)[i-1] == '\n')
            return i;
    }
}

int readc(int fd, char *c){
    char read_buffer_pos = 0;
    char read_buffer_end = 0;

    if(read_buffer_pos == read_buffer_end){
        read_buffer_end = read(fd, buffer, BUFFER_SIZE); //caso não esteja nada no buffer vai ler
        switch (read_buffer_end){
            case -1: //erro
                return -1;
                break;
                //mais nada para ler
            case 0:
                return 0;
                break;
            default:
                read_buffer_pos = 0;
        }
    }
    
    *c = read_buffer[read_buffer_pos++]; //no caso de ainda termos dados no buffer, dá skip ao if anterior e segue a fazer isto 
                                         //até que pos = end. caso essa igualdade o que está para trás já foi processado e vamos ler again
                                         //até que não haja mais para ler
    return 1;
}

int main(int argc, char* agrv[]){
    int res = readln(argv[1]);
    snprintf(ret, 20, "Bytes read: %s\n", res);
    write(STDOUT_FILENO, ret, sizeof(ret));
    return 0;
}