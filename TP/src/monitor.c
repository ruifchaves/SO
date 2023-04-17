#include <unistd.h>     //close
#include <fcntl.h>      //open
#include <sys/types.h>  //fifos, open
#include <sys/stat.h>   //fifos, open
#include <stdio.h>      //sprintf
#include <errno.h>      //errno
#include <stdlib.h>     //exit
#include <string.h>     //strlen, strcmp


#define fifo_cliSer "fifo_client_server"
#define fifo_serCli "fifo_server_client"

int fd_serverClient;

//definir uma lista ligada para ligar as execucoes atuais
//ao ter uma lista ligada allows for constant-time insertion and deletion of elements
typedef struct finishedExec {
    int pid;
    char prog_name[100];
    clock_t start;
    clock_t end;
} *finishedExec;

typedef struct exec {
    int pid;
    char prog_name[100];
    clock_t start;
} exec;

typedef struct llExecs {
    exec exec_info;
    struct llExecs* next;
} llExecs;

llExecs* currExecs;

llExecs* initllExecs() {
    llExecs* head = NULL;
    return head;
}

exec* newExec(int pid, char* name, clock_t start) {
    exec* new = malloc(sizeof(exec));
    new->pid = pid;
    strcpy(new->prog_name, name);
    new->start = start;
    return new;
}

void addExecs(exec* info) {
    llExecs* new = malloc(sizeof(llExecs));
    new->exec_info = *info;
    new->next = currExecs;
    currExecs = new;
}

void removeExecs(int pid){
    llExecs* current = currExecs;
    llExecs* previous = NULL;

    while (current != NULL && current->exec_info.pid != pid) {
        previous = current;
        current = current->next;
    }
    
    if (current == NULL) {
        return;
    }

    if (previous == NULL) {
        currExecs = current->next;
    } else {
        previous->next = current->next;
    }

    free(current);
}

void printllExecs() {
    llExecs* current = currExecs;

    while (current != NULL) {
        printf("PID: %d | Program Name: %s | Start Time: %ld\n", current->exec_info.pid, current->exec_info.prog_name, current->exec_info.start);
        current = current->next;
    }
}

int sizellExecs(){
    int i;
    llExecs* current = currExecs;
    for(i = 0; current != NULL; i++, current=current->next);
    return i;
}






//um execute ao terminal guardar a struct no ficheiro correspondente ao 

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

    currExecs = initllExecs();
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
            if(query_int == 10){  //execute_single
                sprintf(outp, "[REQUEST] New execute request\n");
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
                printf("start received: %ld\n", start_time);
                sprintf(outp, "[EXECUTE Start] PID %d | Command: %s | Start time: %ld\n", pid, prog_name, start_time);
                write(1, &outp, sizeof(char) * strlen(outp));


                addExecs(newExec(pid, prog_name, start_time));

                //collect sent info from request end
                read(fd_clientServer, &pid, sizeof(int));
                clock_t end_time;
                read(fd_clientServer, &end_time, sizeof(clock_t));

                sprintf(outp, "[EXECUTE End]   PID %d | End time: %ld\n", pid, end_time);
                write(1, &outp, strlen(outp) + 1);

                //removeExecs(pid);

            } else if(query_int == 20) { //execute_pipeline
                sprintf(outp, "[REQUEST] New execute request\n");
                write(1, &outp, (strlen(outp) + 1));

                return 0;
            } else if(query_int == 30) { //status
                sprintf(outp, "[REQUEST] New status request\n");
                write(1, &outp, sizeof(outp));

                if(sizellExecs() == 0){
                    sprintf(outp, "[STATUS] That aren't any programs currently running");
                    write(1, &outp, sizeof(char) * strlen(outp));
                } else {
                    int fifo_name_size;
                    read(fd_clientServer, &fifo_name_size, sizeof(int));
                    char fifo_name[fifo_name_size];
                    read(fd_clientServer, &fifo_name, fifo_name_size);
                    printf("debug: fifo_name_size %d %s\n", fifo_name_size, fifo_name);

                    //seguir a abrir e escrever no fifo
                    fd_serverClient = open(fifo_name, O_WRONLY);
                    write(fd_serverClient, "teste", 5);
                }
                //return 0;
            } else if(query_int == 20) { //stats_time
                sprintf(outp, "[REQUEST] New stats_time request\n");
                write(1, &outp, sizeof(outp));
                return 0;
            } else if(query_int == 20) { //stats_command
                sprintf(outp, "[REQUEST] New stats_command request\n");
                write(1, &outp, sizeof(outp));
                return 0;
            } else if(query_int == 20) { //stats_uniq
                sprintf(outp, "[REQUEST] New stats_uniq request\n");
                write(1, &outp, sizeof(outp));
                return 0;
            }
        }
    }
    //close(fd_clientServer);
    unlink(fifoname);
    return 0;
}