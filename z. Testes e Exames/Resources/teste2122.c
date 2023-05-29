#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>


int mensagens(char* palavra, char* ficheiro){
    char buffer[100];
    int resultado=0;
    int br=0;

    int p[2][2];
    pipe(p[0]);
    pipe(p[1]);

    if(fork()==0){
        close(p[0][0]);
        dup2(p[0][1],1);
        execlp("grep","grep",palavra,ficheiro,NULL);
        exit(0);
    }
    wait(NULL);
    close(p[0][1]);

    if(fork()==0){
        close(p[1][0]);
        dup2(p[0][0],0);
        dup2(p[1][1],1);
        execlp("wc","wc","-l",NULL);
        exit(0);
    }
    wait(NULL);
    close(p[0][0]);
    close(p[1][1]);

    br=read(p[1][0],buffer,sizeof(buffer));
    resultado=atoi(buffer);
    close(p[1][0]);

    return resultado;
}

int autores_que_usaram(char* palavra, int n, char* autores[n]){
    pid_t pide[n];
    int resultado=0;

    for (size_t i = 0; i < n; i++)
    {
        if((pide[i]=fork())==0){
            int aux = 0;
            aux = mensagens(palavra,autores[i]);
            exit(aux);
        }
    }

    for (int i = 0; i < n; i++)
    {
        int status;
        pid_t aux = waitpid(pide[i],&status,0);
        if(WIFEXITED(status)){
            if(WEXITSTATUS(status))resultado++;
        }
        else{
            char buffer[256];
            int bytes = sprintf(buffer, "Erro no ficheiro: %d\n", i);
            write(2, buffer, bytes);
        }
    }

    return resultado;
} 