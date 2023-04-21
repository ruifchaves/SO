#include <unistd.h>     //close
#include <fcntl.h>      //open
#include <sys/types.h>  //fifos, open
#include <sys/stat.h>   //fifos, open
#include <stdio.h>      //sprintf
#include <errno.h>      //errno
#include <stdlib.h>     //exit
#include <string.h>     //strlen, strcmp
#include <time.h>       //clock_t, clock
#include <sys/time.h>   //gettimeofday
#include <sys/wait.h>   //wait


#define fifo_cliSer "fifo_client_server"
#define fifo_serCli "fifo_server_client"

int fd_serverClient;
int llexec_size;
int llfin_size;
int request_id = 0;

//definir uma lista ligada para ligar as execucoes atuais
//ao ter uma lista ligada allows for constant-time insertion and deletion of elements
typedef struct exec {
    int pid;
    char prog_name[100];
    struct timeval start, end;
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

exec* add_exec(int pid, char* name, struct timeval start) {
    exec* new = malloc(sizeof(exec));
    new->pid = pid;
    strcpy(new->prog_name, name);
    new->start = start;

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
        struct timeval start = current->exec_info.start;
        long start_ms = start.tv_sec * 1000 + start.tv_usec / 1000;
        sprintf(outp, "[Running Program #%d] PID %d | Program Name %s | Start timeval: %ld\n", i, current->exec_info.pid, current->exec_info.prog_name, start_ms);
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
        struct timeval start = current->exec_info.start;
        struct timeval end = current->exec_info.end;
        long start_ms = start.tv_sec * 1000 + start.tv_usec / 1000;
        long end_ms = end.tv_sec * 1000 + end.tv_usec / 1000;
        sprintf(outp, "[Finished Program #%d] PID %d | Program Name %s | Start Time %ld | End Time %ld\n", i, current->exec_info.pid, current->exec_info.prog_name, start_ms, end_ms);
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

            //how to pass current->exec_info.start value to parent process??

            //int elapsed_time = calculate_elapsed_time(current->exec_info.start);
            _exit(0);// - (int) current->exec_info.start);
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
                int store_request_id = request_id++;
                sprintf(outp, "[REQUEST #%d] New execute single request\n", store_request_id);
                write(1, &outp, strlen(outp));

                //collect sent info from new request
                int pid;
                int prog_name_size;
                struct timeval start_time;
                read(fd_clientServer, &pid, sizeof(int));
                read(fd_clientServer, &prog_name_size, sizeof(int));
                char prog_name[prog_name_size+1];
                read(fd_clientServer, &prog_name, prog_name_size * sizeof(char));
                prog_name[prog_name_size] = '\0'; //add null terminator
                read(fd_clientServer, &start_time, sizeof(struct timeval));
                
                long start_time_ms = start_time.tv_sec * 1000 + start_time.tv_usec / 1000;
                sprintf(outp, "[EXECUTE Start] PID %d | Command: \"%s\" | Start timeval: %ld\n", pid, prog_name, start_time_ms);
                write(1, &outp, sizeof(char) * strlen(outp));

                //exec* prog_exec = newExec(pid, prog_name, start_time);
                add_exec(pid, prog_name, start_time);

                //collect sent info from request end
                read(fd_clientServer, &pid, sizeof(int));
                struct timeval end_time;
                read(fd_clientServer, &end_time, sizeof(struct timeval));

                long end_time_ms = end_time.tv_sec * 1000 + end_time.tv_usec / 1000;
                long elapsed_seconds = end_time.tv_sec - start_time.tv_sec;
                long elapsed_useconds = end_time.tv_usec - start_time.tv_usec;
                long elapsed_time = (elapsed_seconds * 1000) + (elapsed_useconds / 1000);

                sprintf(outp, "[EXECUTE End]   PID %d | End timeval: %ld | Total time: %ld ms\n", pid, end_time_ms, elapsed_time);
                write(1, &outp, strlen(outp));

                remove_exec(pid);
                close(fd_serverClient);


            } else if(query_int == 20) { //execute_pipeline
                int store_request_id = request_id++;
                sprintf(outp, "[REQUEST #%d] New execute pipeline request\n", store_request_id);
                write(1, &outp, strlen(outp));

                //collect sent info from new request
                int pid;
                int prog_name_size;
                struct timeval start_time;
                read(fd_clientServer, &pid, sizeof(int));
                read(fd_clientServer, &prog_name_size, sizeof(int));
                char prog_name[prog_name_size+1];
                read(fd_clientServer, &prog_name, prog_name_size * sizeof(char));
                prog_name[prog_name_size] = '\0'; //add null terminator
                read(fd_clientServer, &start_time, sizeof(struct timeval));
                
                long start_time_ms = start_time.tv_sec * 1000 + start_time.tv_usec / 1000;
                sprintf(outp, "[EXECUTE Start] PID %d | Command: \"%s\" | Start timeval: %ld\n", pid, prog_name, start_time_ms);
                write(1, &outp, sizeof(char) * strlen(outp));

                //exec* prog_exec = newExec(pid, prog_name, start_time);
                add_exec(pid, prog_name, start_time);

                //collect sent info from request end
                read(fd_clientServer, &pid, sizeof(int));
                struct timeval end_time;
                read(fd_clientServer, &end_time, sizeof(struct timeval));

                long end_time_ms = end_time.tv_sec * 1000 + end_time.tv_usec / 1000;
                long elapsed_seconds = end_time.tv_sec - start_time.tv_sec;
                long elapsed_useconds = end_time.tv_usec - start_time.tv_usec;
                long elapsed_time = (elapsed_seconds * 1000) + (elapsed_useconds / 1000);

                sprintf(outp, "[EXECUTE End]   PID %d | End timeval: %ld | Total time: %ld ms\n", pid, end_time_ms, elapsed_time);
                write(1, &outp, strlen(outp));

                remove_exec(pid);
                close(fd_serverClient);

            } else if(query_int == 30) { //status
                int store_request_id = request_id++;
                sprintf(outp, "[REQUEST #%d] New status request\n", store_request_id);
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
                        struct timeval til_now;
                        gettimeofday(&til_now, NULL);
                        struct timeval start = tmp->exec_info.start;

                        long elapsed_seconds = til_now.tv_sec - start.tv_sec;
                        long elapsed_useconds = til_now.tv_usec - start.tv_usec;
                        long elapsed_time = (elapsed_seconds * 1000) + (elapsed_useconds / 1000);   

                        sprintf(outp, "%d %s %ld ms\n", tmp->exec_info.pid, tmp->exec_info.prog_name, elapsed_time);
                        int exec_size = strlen(outp);
                        write(fd_serverClient, &exec_size, sizeof(int));
                        write(fd_serverClient, &outp, exec_size);
                    }
                }

                print_llexec(); //debug
                print_llfin();  //debug





            } else if(query_int == 40) { //stats_time
                int store_request_id = request_id++;
                sprintf(outp, "[REQUEST #%d] New stats_time request\n", store_request_id);
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

                    //enviar string de output
                    //sprintf(outp, "Total execution time is %f ms\n", cpu_time_used);
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