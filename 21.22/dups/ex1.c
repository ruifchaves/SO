#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

//redirecionamento de um fich txt para o stdin: cat < fich.txt
int main(int argc, char* argv[]){
    int res = 0;
    int i = 0;
    char buffer;
    char line[1024];

    int stdin_fd  = open ("/etc/passwd", O_RDONLY);
    int stdout_fd = open ("saida.txt", O_CREAT | O_TRUNC | O_WRONLY, 0666);
    int error_fd  = open ("error.txt", O_CREAT | O_TRUNC | O_WRONLY, 0666);

    res = dup2 (stdin_fd, 0);
    printf("inputfd(old, new) = %d, %d", stdin_fd, res);
    res = dup2 (stdout_fd, 1);
    printf("outputfd(%d, %d)", stdout_fd, res);
    res = dup2 (error_fd, 2);
    printf("errorfd(%d, %d)", error_fd, res);

    close(stdin_fd);
    close(stdout_fd);
    close(error_fd);

    int read_res;
    while((read_res = read(0, &buffer, 1)) != 0){
        line[i] = buffer;
        i++;

        if (buffer == '\n'){
            write(1, line, i);
            write(2, line, i);

            printf("after write line\n");

            //TODO ao utilizar stdio necessário para fazer flush
            fflush(stdout);
            //ou fora do ciclo setbuf(stdout, NULL);
            i=0;
        }
    }
    //prinf() ou write("terminei")
    return 0;
}