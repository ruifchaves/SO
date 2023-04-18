#include <unistd.h>     //close
#include <fcntl.h>      //open
#include <sys/types.h>  //fifos, open
#include <sys/stat.h>   //fifos, open
#include <stdio.h>      //sprintf
#include <errno.h>      //errno
#include <stdlib.h>     //exit
#include <string.h>     //strlen, strcmp
#include <time.h>       //clock_t, clock


#define fifo_cliSer "fifo_client_server"
#define fifo_serCli "fifo_server_client"

int fd_serverClient;
int llExecs_size;


//definir uma lista ligada para ligar as execucoes atuais
//ao ter uma lista ligada allows for constant-time insertion and deletion of elements
typedef struct exec {
    int pid;
    char prog_name[100];
    clock_t start;
    clock_t end;
} exec;

typedef struct llExecs {
    exec exec_info;
    struct llExecs* next;
} llExecs;


llExecs* currExecs;

llExecs* init_llExecs() {
    llExecs* head = NULL;
    return head;
}

exec* addExec(int pid, char* name, clock_t start) {
    exec* new = malloc(sizeof(exec));
    new->pid = pid;
    strcpy(new->prog_name, name);
    new->start = start;

    llExecs* node = malloc(sizeof(llExecs));
    node->exec_info = *new;
    node->next = currExecs;
    currExecs = node;

    return new;
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
        exit(-1);
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
    int i = 1;
    while (current != NULL) {
        char outp[200];
        sprintf(outp, "[Running Program #%d] PID %d | Program Name %s | Start Time %ld\n", i, current->exec_info.pid, current->exec_info.prog_name, current->exec_info.start);
        write(1, &outp, strlen(outp));
        current = current->next;
        i++;
    }
}

int sizellExecs(){
    int i;
    llExecs* current = currExecs;
    for(i = 0; current != NULL; i++, current=current->next);
    return i;
}

/////////////////////// HASH TABLE //////////////////////

#define HASH_TABLE_SIZE 100

typedef struct finished_exec {
    int pid;
    double start_time;
    double end_time;
    struct finished_exec* next;
} finished_exec;

typedef struct hash_table {
    finished_exec** table;
} hash_table;

hash_table* htabFin;

int hash(int pid) {
    return pid % HASH_TABLE_SIZE;
}

void init_htabFin() {
    htabFin = (hash_table*) malloc(sizeof(hash_table));
    htabFin->table = (finished_exec**) malloc(HASH_TABLE_SIZE * sizeof(finished_exec*));
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        htabFin->table[i] = NULL;
    }
}

void insert_finished_exec(int pid, double start, double end) {
    finished_exec* new_exec = (finished_exec*) malloc(sizeof(finished_exec));
    new_exec->pid = pid;
    new_exec->start_time = start;
    new_exec->end_time = end;
    new_exec->next = NULL;
    
    int index = hash(pid);
    finished_exec* current = htabFin->table[index];
    if (current == NULL) {
        htabFin->table[index] = new_exec;
    } else {
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_exec;
    }
}

finished_exec* search_finished_exec(int pid) {
    int index = hash(pid);
    finished_exec* current = htabFin->table[index];
    while (current != NULL) {
        if (current->pid == pid) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void print_hash_table() {
    char outp[100];
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        finished_exec* current = htabFin->table[i];
        while (current != NULL) {
            sprintf(outp, "[Finished Program] PID %d | Start Time %.2lf | End Time %.2lf\n", current->pid, current->start_time, current->end_time);
            write(1, &outp, strlen(outp));
            current = current->next;
        }
    }
}


//um execute ao terminal guardar a struct no ficheiro correspondente ao 

int main(int argc, char* agrv[]){
    char fifoname[50];
    int res_createfifo;
    int fd_clientServer;
    int res_readfifo;
    char fifo_in[100];
    char outp[200];
    //llFinishedExecs_size = 0;

    //criar pipe para leitura do escrito pelos clientes
    sprintf(fifoname, "../tmp/%s", fifo_cliSer);
    res_createfifo = mkfifo(fifoname, 0660);
    if(res_createfifo == -1){
        if(errno != EEXIST) {
            perror("Error creating Client->Server pipe");
            exit(-1);
        }
    }

    init_htabFin();
    currExecs = init_llExecs();
    //finExecs = initllFinishedExecs();
    while(1){
        //Abrir o pipe criado
        print_hash_table();
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
                addExec(pid, prog_name, start_time);
                insert_finished_exec(pid, start_time, 0.0);

                //collect sent info from request end
                read(fd_clientServer, &pid, sizeof(int));
                clock_t end_time;
                read(fd_clientServer, &end_time, sizeof(clock_t));

                sprintf(outp, "[EXECUTE End]   PID %d | End time: %ld\n", pid, end_time);
                write(1, &outp, strlen(outp));

                //removeExecs(pid);
                //addFinExecs(prog_exec, end_time);

            } else if(query_int == 20) { //execute_pipeline
                sprintf(outp, "[REQUEST] New execute request\n");
                write(1, &outp, strlen(outp));

                return 0;
            } else if(query_int == 30) { //status
                //char outp[200];
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
                int sizell = sizellExecs();
                write(fd_serverClient, &sizell, sizeof(int));
                if(sizell == 0){
                    sprintf(outp, "[STATUS] That aren't any programs currently running\n");
                    write(1, &outp, strlen(outp));
                } else {
                    llExecs* tmp = currExecs;
                    for(llExecs* tmp = currExecs; tmp != NULL; tmp=tmp->next){
                        sprintf(outp, "%d %s %ld ms\n", tmp->exec_info.pid, tmp->exec_info.prog_name, tmp->exec_info.start);
                        int exec_size = strlen(outp);
                        write(fd_serverClient, &exec_size, sizeof(int));
                        write(fd_serverClient, &outp, exec_size);
                    }
                }
                printllExecs();
                //printllFinishedExecs();
                //return 0;
            } else if(query_int == 40) { //stats_time
                sprintf(outp, "[REQUEST] New stats_time request\n");
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



                //int size = search_pid_finishedExecs();
                //write()


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