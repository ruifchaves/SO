#include <unistd.h>     //close
#include <fcntl.h>      //open
#include <sys/types.h>  //fifos, open
#include <sys/stat.h>   //fifos, open
#include <stdio.h>      //sprintf
#include <errno.h>      //errno
#include <stdlib.h>     //exit
#include <string.h>     //strlen, strcmp


#define fifo_cliSer "fifo_client_server"

int main(int argc, char* agrv[]){
    char fifoname[50];
    int res_createfifo;
    int fd_clientServer;
    int res_readfifo;
    char fifo_in[100];
    char outp[100];

    //criar pipe para leitura do escrito pelos clientes
    sprintf(fifoname, "../tmp/%s", fifo_cliSer);
    res_createfifo = mkfifo(fifoname, 0660);
    if(res_createfifo == -1){
        if(errno != EEXIST) {
            perror("Error creating Client->Server pipe");
            exit(-1);
        }
    }
 
    while(1){
        //Abrir o pipe criado
        fd_clientServer = open(fifoname, O_RDONLY);
        if(fd_clientServer == -1){
            perror("Error opening Client->Server pipe to read");
            exit(-1);
        }

        //pipe opened
        int query_int;
        while((res_readfifo = read(fd_clientServer, &query_int, sizeof(int))) > 0){
            if(query_int == 10){
                sprintf(outp, "[REQUEST] New request\n");
                write(1, &outp, sizeof(outp));
                //collect sent info from new request
                int pid;
                int prog_name_size;
                clock_t start_time;
                read(fd_clientServer, &pid, sizeof(int));
                read(fd_clientServer, &prog_name_size, sizeof(int));
                char prog_name[prog_name_size+1];
                read(fd_clientServer, &prog_name, prog_name_size * sizeof(char));
                prog_name[prog_name_size] = '\0'; //add null terminator
                read(fd_clientServer, &start_time, sizeof(clock_t));
                sprintf(outp, "[REQUEST] PID %d | Command: %s | Start time: %ld\n", pid, prog_name, start_time);
                write(1, &outp, sizeof(char) * strlen(outp));


                read(fd_clientServer, &fifo_in, 8);
                write(1, &fifo_in, 8);

            } else if(query_int == 0) {
                return 0;
            }
        }
    }
    //close(fd_clientServer);
    unlink(fifoname);
    return 0;
}