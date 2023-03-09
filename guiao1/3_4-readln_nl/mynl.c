#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>


ssize_t readln(int fd, char* line, size_t size){
    size_t i = 0;

    for (; i < size && read(fd, &line[i], 1) > 0; i++) {
        if (line[i] == '\n') //if (((char*) line)[i-1] == '\n')
            return i;
    }


    return i;
}


int main(){
    size_t size_bytes = 20;
    int i = 1;
    
    while (1){
        
        char line[size_bytes];

        if (readln(STDIN_FILENO, line, size_bytes) != 0){
            size_t ret_size = 40;
            char answer[ret_size];
            snprintf(answer,ret_size,"     %d  %s", i, line);
            write(STDOUT_FILENO, answer, sizeof(answer));
            i++;
        } else {
            return 1;
        }
    }

    return 0;
}