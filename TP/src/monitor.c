#include <unistd.h>     //close
#include <fcntl.h>      //open
#include <sys/types.h>  //fifos, open
#include <sys/stat.h>   //fifos, open
#include <stdio.h>      //sprintf
#include <errno.h>      //errno
#include <stdlib.h>     //exit
#include <string.h>     //strlen, strcmp
#include <time.h>       //clock_t, clock
#include <sys/wait.h>   //wait


#define fifo_cliSer "fifo_client_server"
#define fifo_serCli "fifo_server_client"

int fd_serverClient;
int llexec_size;
int llfin_size;

//definir uma lista ligada para ligar as execucoes atuais
//ao ter uma lista ligada allows for constant-time insertion and deletion of elements
typedef struct exec {
    int pid;
    char prog_name[100];
    clock_t start;
    clock_t end;
} exec;

typedef struct llexec {
    exec exec_info;
    struct llexec* next;
} llexec;

typedef struct llfin {
    exec exec_info;
    struct llfin* next;
} llfin;


llexec* currExecs;
llfin* finExecs;


llexec* init_llexec() {
    llexec* head = NULL;
    return head;
}

llfin* init_llfin() {
    llfin* head = NULL;
    return head;
}

exec* add_exec(int pid, char* name, clock_t start) {
    exec* new = malloc(sizeof(exec));
    new->pid = pid;
    strcpy(new->prog_name, name);
    new->start = start;
    new->end = 0.0;

    llexec* node = malloc(sizeof(llexec));
    node->exec_info = *new;
    node->next = currExecs;
    currExecs = node;

    return new;
}

void add_finExec(exec* finished){
    llfin* node = malloc(sizeof(llfin));
    node->exec_info = *finished;
    node->next = finExecs;
    finExecs = node;

    llfin_size++;
}

void remove_exec(int pid){
    llexec* current = currExecs;
    llexec* previous = NULL;

    while (current != NULL && current->exec_info.pid != pid) {
        previous = current;
        current = current->next;
    }
    
    if (current == NULL) {
        exit(-1);
    }

    if (previous == NULL) {
        currExecs = current->next;
    } else {
        previous->next = current->next;
    }

    add_finExec(&current->exec_info);

    free(current);
}

void print_llexec() {
    llexec* current = currExecs;
    int i = 1;
    while (current != NULL) {
        char outp[200];
        sprintf(outp, "[Running Program #%d] PID %d | Program Name %s | Start Time %ld\n", i, current->exec_info.pid, current->exec_info.prog_name, current->exec_info.start);
        write(1, &outp, strlen(outp));
        current = current->next;
        i++;
    }
}

void print_llfin() {
    llfin* current = finExecs;
    int i = 1;
    while (current != NULL) {
        char outp[200];
        sprintf(outp, "[Finished Program #%d] PID %d | Program Name %s | Start Time %ld | End Time %ld\n", i, current->exec_info.pid, current->exec_info.prog_name, current->exec_info.start, current->exec_info.end);
        write(1, &outp, strlen(outp));
        current = current->next;
        i++;
    }
}

int size_llexec(){
    int i;
    llexec* current = currExecs;
    for(i = 0; current != NULL; i++, current=current->next);
    return i;
}

int size_llfin(){
    int i;
    llfin* current = finExecs;
    for(i = 0; current != NULL; i++, current=current->next);
    return i;
}




int search_pid_finishedExecs(int* pids, int pids_size){

    for(int i = 0; i < pids_size; i++) printf("pids[i] %d\n", pids[i]);
    llfin* current = finExecs;
    int total_time = 0;
    int status;

    //comparar os varios pids em cada fork/elemento da ll  concorrentemente
    //aka comparacao se algum dos pids passados em pids é o elem
    //em vez de comparar todos os pids no current e fazer o mesmo no seguinte
    for(int i=0; i < llfin_size; i++, current=current->next){
        int resf = fork();
        if(resf == 0){
            int llelem_pid = current->exec_info.pid;
            int i;
            for(i = 0; llelem_pid != pids[i] && i < pids_size; i++);
            if(i == pids_size) _exit(0);

            _exit((clock_t) current->exec_info.start);// - (int) current->exec_info.start);
        }
    }

    for(int i = 0; i < llfin_size; i++){
        wait(&status);
        printf("WEXITSTATUS %d: %d\n", i, WEXITSTATUS(status));
        total_time += WEXITSTATUS(status);
        //printf("total time: %d\n", total_time);
    }

    return total_time;
}


int main(int argc, char* agrv[]){
    char fifoname[50];
    int res_createfifo;
    int fd_clientServer;
    int res_readfifo;
    char fifo_in[100];
    char outp[200];
    llexec_size = 0;
    llfin_size = 0;

    //criar pipe para leitura do escrito pelos clientes
    sprintf(fifoname, "../tmp/%s", fifo_cliSer);
    res_createfifo = mkfifo(fifoname, 0660);
    if(res_createfifo == -1){
        if(errno != EEXIST) {
            perror("Error creating Client->Server pipe");
            exit(-1);
        }
    }

    currExecs = init_llexec();
    finExecs = init_llfin();
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
                write(1, &outp, strlen(outp));

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
                sprintf(outp, "[EXECUTE Start] PID %d | Command: %s | Start time: %ld\n", pid, prog_name, start_time);
                write(1, &outp, sizeof(char) * strlen(outp));

                //exec* prog_exec = newExec(pid, prog_name, start_time);
                add_exec(pid, prog_name, start_time);

                //collect sent info from request end
                read(fd_clientServer, &pid, sizeof(int));
                clock_t end_time;
                read(fd_clientServer, &end_time, sizeof(clock_t));

                sprintf(outp, "[EXECUTE End]   PID %d | End time: %ld\n", pid, end_time);
                write(1, &outp, strlen(outp));

                remove_exec(pid);
                close(fd_serverClient);






            } else if(query_int == 20) { //execute_pipeline
                sprintf(outp, "[REQUEST] New execute request\n");
                write(1, &outp, strlen(outp));

                return 0;




            } else if(query_int == 30) { //status
                sprintf(outp, "[REQUEST] New status request\n");
                write(1, &outp, strlen(outp));

                //receber o nome do fifo criado
                int fifo_name_size;
                read(fd_clientServer, &fifo_name_size, sizeof(int));
                char fifo_name[fifo_name_size];
                read(fd_clientServer, &fifo_name, fifo_name_size);

                ////abrir o fifo entre servidor e cliente
                fd_serverClient = open(fifo_name, O_WRONLY);
                if(fd_serverClient == -1){
                    perror("Error opening Server->Client pipe to write");
                    exit(-1);
                }

                //verificar se há algum programa em execução
                int sizell = size_llexec();
                write(fd_serverClient, &sizell, sizeof(int));
                if(sizell == 0){
                    sprintf(outp, "[STATUS] That aren't any programs currently running\n");
                    write(1, &outp, strlen(outp));
                } else {
                    llexec* tmp = currExecs;
                    for(llexec* tmp = currExecs; tmp != NULL; tmp=tmp->next){
                        sprintf(outp, "%d %s %ld ms\n", tmp->exec_info.pid, tmp->exec_info.prog_name, tmp->exec_info.start);
                        int exec_size = strlen(outp);
                        write(fd_serverClient, &exec_size, sizeof(int));
                        write(fd_serverClient, &outp, exec_size);
                    }
                }

                print_llexec(); //debug
                print_llfin();  //debug





            } else if(query_int == 40) { //stats_time
                sprintf(outp, "[REQUEST] New stats_time request\n");
                write(1, &outp, strlen(outp));

                //receber o nome do fifo criado
                int fifo_name_size;
                read(fd_clientServer, &fifo_name_size, sizeof(int));
                char fifo_name[fifo_name_size];
                read(fd_clientServer, &fifo_name, fifo_name_size);

                //abrir o fifo entre servidor e cliente
                fd_serverClient = open(fifo_name, O_WRONLY);                    
                if(fd_serverClient == -1){
                    perror("Error opening Server->Client pipe to write");
                    exit(-1);
                }

                //verificar se há algum programa que já acabou
                int sizell = size_llfin();
                write(fd_serverClient, &sizell, sizeof(int));
                if(sizell == 0){
                    sprintf(outp, "[STATS_TIME] That aren't any programs that have already finished\n");
                    write(1, &outp, strlen(outp));
                } else {
                    //ler pids e pids_size
                    int pids_size;
                    read(fd_clientServer, &pids_size, sizeof(int));
                    int pids[pids_size];
                    for(int i = 0; i < pids_size; i++){
                        int tmp;
                        read(fd_clientServer, &tmp, sizeof(int));
                        pids[i] = tmp;
                        //printf("pids received: %d\n", pids[i]);
                    }
                    
                    //calcular tempo total
                    int total_time = search_pid_finishedExecs(pids, pids_size);
                    printf("current->exec_info.end: %d\n", total_time);
                    double cpu_time_used = (((double) total_time) / CLOCKS_PER_SEC) * 1000;

                    //enviar string de output
                    sprintf(outp, "Total execution time is %f ms\n", cpu_time_used);
                    int tot_size = strlen(outp);
                    write(fd_serverClient, &tot_size, sizeof(int));
                    write(fd_serverClient, &outp, tot_size);
                }






            } else if(query_int == 50) { //stats_command
                sprintf(outp, "[REQUEST] New stats_command request\n");
                write(1, &outp, strlen(outp));
                return 0;
            } else if(query_int == 60) { //stats_uniq
                sprintf(outp, "[REQUEST] New stats_uniq request\n");
                write(1, &outp, strlen(outp));
                return 0;
            }
        }
    }
    //close(fd_clientServer);
    unlink(fifoname);
    return 0;
}