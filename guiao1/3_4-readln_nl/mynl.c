#include "mynl.h"


//3. Implemente a leitura de uma linha (i.e. sequência terminada por \n) numa função readln.
//4. Implemente, utilizando a função readln, um programa com funcionalidade similar ao comando nl,
//que numera as linhas recebidas no seu standard input.
ssize_t readln(int fd, char *line, size_t size){
    ssize_t i = 0;

    for(; i < size && read(fd, &line[i], 1) > 0; i++){
        if (((char*) line)[i] == '\n')
            return i;
    } if(i >= size) return -1; //so it doesnt return 0
    
    return 0;
}

int main(){
    int i = 1;

    while(1){
        char line[LINE_SIZE];
        memset(line, 0, LINE_SIZE); //!! any remaining chars from previous input are cleared

        size_t ret_size = 35; //has to be > than line's size
        char ret[ret_size];

        int len = readln(STDIN_FILENO, line, LINE_SIZE);
        if(len != 0){
            int size = snprintf(ret, ret_size,"     %5d %s", i, line);
            write(STDOUT_FILENO, ret, size); //não pode ser sizeof(ret) em vez de size. size: tam do que foi lido; sizeof: tamanho do buffer 20...
            //ret value should be checked

            i++;

        } else if(len == 0){ //enter sem escrita de chars, double line, no increment
            write(STDOUT_FILENO, "\n", 1); //ret value should be checked
        } 
        //else if(len == -1){
        //    write(STDOUT_FILENO, "           Buffer too small...", 30);
        //    i++;
        //}
    }
    
    return 0;
}