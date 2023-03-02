#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>


//./a.out ls ps ping
int main(int argc, char** argv){
    pid_t pid;
    int status, return_exec;


    if(argc == 1){
        return -1;
    } else {
        for(int i = 1; i < argc; i++){
            if((pid = fork()) == 0) {
                //codigo processo filho
                printf("[pid-filho %d: started | going to execute command %s]\n", getpid(), argv[i]);
                //Atenção ao usar o execvp
                //return_exec = execvp (argv[i], &argv[i]);

                return_exec = execlp (argv[i], argv[i], NULL);
                printf("[pid-filho %d: after exec]\n", getpid());

                perror("reached return");
                _exit (return_exec); //retorna valor gerado pelo binario
            }
        }
    }


    for (int i = 1; i < argc; i++){
        pid_t terminated_pid = wait(&status);

        if(WIFEXITED(status)) {
            printf("[pid-pai %d: ended with Exit Status %d]\n", terminated_pid, WEXITSTATUS(status));
        } else {
            printf("pai: o processo filho não terminou\n");
        }
    }
    return 0;
}



