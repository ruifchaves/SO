#include "rl.h"


//vars adicionais para ultima implemetacao de readc
#define MAX_READ_BUFFER 2048
char read_buffer[MAX_READ_BUFFER]
char read_buffer_pos = 0
char read_buffer_end = 0




//ex 3
//\n incluido no buffer de resposta
ssize_t readln(int fd, char *line, size_t size){
    int i = 0;
    while( i < size && readc(fd, &buf[i]) > 0){
        i++;
        if (((char*) buf)[i-1] == '\n'){
            return i;
        }
    }
}

//ex 4
int readc(int fd, char *c){
    //implementação char a char
    //return read(fd, c, 1);

    if (read_buffer_pos == read_buffer_end){
        read_buffer_end = read(fd, read_buffer, MAX_READ_BUFFER); //caso não esteja nada no buffer vai ler
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
