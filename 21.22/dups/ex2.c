#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

//redirecionamento de um fich txt para o stdin: cat < fich.txt
int main(int argc, char* argv[]){
    int res = 0;
    int i = 0;
    char buffer;
    char line[1024];

    setbuf(stdout, NULL);
    int fdin = dup(0)
    int fdout = dup(1);

    int stdin_fd  = open ("/etc/passwd", O_RDONLY);
    int stdout_fd = open ("saida.txt", O_CREAT | O_TRUNC | O_WRONLY, 0666);
    int error_fd  = open ("error.txt", O_CREAT | O_TRUNC | O_WRONLY, 0666);

    res = dup2 (stdin_fd, 0);
    res = dup2 (stdout_fd, 1);
    res = dup2 (error_fd, 2);

    
    close(stdin_fd);
    close(stdout_fd);
    close(error_fd);

    int read_res;

    if((pid_t pid = fork()) == 0){
        printf("sou o filho: %d", getpid());

        /*
        while((read_res = read(0, &buffer, 1)) != 0){
            line[i] = buffer;
            i++;

            if (buffer == '\n'){
                write(1, line, i);
                write(2, line, i);

                printf("after write line\n");

                //ao utilizar stdio necessário para fazer flush
                //fflush(stdout);
                i=0;
            }
        }*/ não tenho a certeza do que está comment, igual ao anterior
        _exit(0)

    } else {
        printf("sou o pai: %d", getpid());
        pid_t terminated_pid = wait(&status);
        
        dup2(fdin, 0);
        dup2(fdout, 1);

        if(WIFEXITED(status)) {
            printf("o filho retornou %d\n", WEXITSTATUS(status));
        } else {
            printf("o processo filho não terminou\n");
        }
    }
    return 0;
}