#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>


int main (int argc, char* agrv[]){
    int filds[2];
    int res = pipe(filds);



    if((pid = fork())==0){ //consumidor, filho
        close(filds[1]); //esta pronto a ler, so falta consumir
        read(filds[0],...,...);
        close(filds[0]);
        _exit(...);

    } else { //produtor, pai
        close(filds[0]);
        write(filds[1], "...", ...);
        close(filds[1]);
        wait();

    }
}


switch(fork()){
    case -1:
        perror("Fork not made");
    case 0:
        //codigo filho
        //fechar extremidade de leitura
        close(p[1]);

        //ler do pipe
        read("[FILHO] ", , );

        //fechar extremidade de leitura
        close(p[0]);
        _exit(0);
    default:
        //codigo pai
        //fechar extremidade de leitura
        close(p[0]);

        //escrever
        write(p[1], &line, strlen(line));

        //fechar extremidade de escrita (nao preciso mais dele)
        close(p[1]);
        wait(p[0]);
}