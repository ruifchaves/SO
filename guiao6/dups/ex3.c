#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

//wc < /etc/passwd > saida.txt
//wc -> word count, terminar com ctrl+d

int main(int argc, char* argv[]){
    int res = 0;
    int i = 0;
    char buffer;
    char line[1024];

    int fdin    = open ("/etc/passwd", O_RDONLY);
    int fdout   = open ("saida.txt", O_CREAT | O_TRUNC | O_WRONLY, 0666);
    //int fderror = open ("error.txt", O_CREAT | O_TRUNC | O_WRONLY, 0666);

    dup2 (fdin, 0);
    dup2 (fdout, 1);
    //dup2 (error_fd, 2);

    close(fdin);
    close(fdout);
    //close(error_fd);
    
    int return_exec;
    //keep in mind
    return_exec = execlp("wc", "wc", NULL);
    
    
    return 0;
}