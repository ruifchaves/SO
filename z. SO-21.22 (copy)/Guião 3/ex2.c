#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

int main (int argc, char** agrv){
    
    pid_t pid;
    int status, return_exec;
    //char* exec_args[] = {"/bin/ls", "ls", NULL};

    printf("main-process: msg 1\n");

    if((pid = fork()) == 0){
        //codigo processo filho
        return_exec = execlp("ls", "ls", "-l", NULL);
        //return_exec = execvp("ls", exec_args);

        printf("processo filho: depois do exec...\n");
        _exit(return_exec);
        
    } else {
        //codigo processo pai
        pid_t terminated_pid = wait(&status);

        if(WIFEXITED(status)) {
            printf("pai: processo %d acabou com o resultado %d\n", terminated_pid, WEXITSTATUS(status));
        } else {
            printf("pai: o processo filho não terminou\n");
        }
    
        printf("pai: msg2\n");
    }
    return 0;
}