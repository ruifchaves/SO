#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

//int system(const char *command)<
//return -1 se fork falha senao retorna valor do comando executado
int mysystem (char* command){

    pid_t pid;
    int return_exec, status, return_value;

    //Estamos a assumir numero maximo de argumentos, teria de ser melhorado com realloc
    char* exec_args[20];
    char* string;
    int i = 0;

    //char* strtok (char* str, const char* delim)
    string = strtok(command, " ");

    while(string != NULL){
        exec_args[i] = string;
        string = strtok (NULL, " ");
        i++;
    }

    exec_args[i] = NULL;

    //Sem fork o programa principal era trocado...
    if ((pid = fork()) == 0) {
        return_exec = execvp (exec_args[0], exec_args);
        _exit(return_exec);
    } else {
        if (pid != -1) {
            pid_t terminated_pid = waitpid (pid, &status, 0);

            if(WIFEXITED (status)) {
                return_value = WEXITSTATUS (status);
            } else {
                return_value = -1;
            }
        } else {
            return_value = -1;
        }
    }

    return return_value;
}

int main(int argc, char** argv) {

    char comando1[] = "ls -l -a -h";
    char comando2[] = "sleep 10";
    char comando3[] = "ps";
    int ret;

    printf("a executar mysystem para %s\n", comando1);
    ret = mysystem(comando1);
    printf("Return Value: %d\n\n", ret);

    printf("a executar mysystem para %s\n", comando2);
    ret = mysystem(comando2);
    printf("Return Value: %d\n\n", ret);
    
    printf("a executar mysystem para %s\n", comando3);
    ret = mysystem(comando3);
    printf("Return Value: %d\n\n", ret);

    return 0;
}
