#include <unistd.h>     //close
#include <fcntl.h>      //open
#include <sys/types.h>  //fifos, open
#include <sys/stat.h>   //fifos, open
#include <stdio.h>      //sprintf
#include <errno.h>      //errno
#include <stdlib.h>     //exit
#include <string.h>     //strlen, strcmp
#include <sys/time.h>   //gettimeofday
#include <sys/wait.h>   //wait


#define fifo_cliSer "fifo_client_server"
#define fifo_serCli "fifo_server_client"

int fd_serverClient;
int llexec_size;
int llfin_size;
int request_id = 0;
char fin_dir[50];

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

void add_finExec(exec* finished, struct timeval end, long elapsed){
    llfin* node = malloc(sizeof(llfin));
    node->exec_info = *finished;
    node->exec_info.end = end;
    //node->exec_info.elapse = elapsed;
    node->next = finExecs;
    finExecs = node;

    llfin_size++;
}

void add_to_file_finished(int pid, char* folder){
    llfin* current = finExecs;

    char folder_file[100];
    sprintf(folder_file, "%s%d.txt", fin_dir, pid);
    int open_res = open(folder_file, O_CREAT | O_WRONLY, 0660);
    if (open_res < 0){
        perror("Error opening file that stores information of finished execution");
        exit(-1);
    }

    int i = 0;
    for(i=0; current->exec_info.pid != pid && i < llfin_size; i++, current=current->next);
    if(i == llfin_size){
        perror("Finished execution not found");
        exit(-1);
    } else {

        //escrever a struct e depois cada valor (so thats readable), para poder pesquisar depois
        write(open_res, &current->exec_info, sizeof(struct exec));
        char tmp[100];
        sprintf(tmp, "\n\n");
        write(open_res, &tmp, strlen(tmp));


        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wformat"

        int pid = current->exec_info.pid;
        int prog_name_size = strlen(current->exec_info.prog_name);
        char prog_name[prog_name_size];
        sprintf(prog_name, "%s", current->exec_info.prog_name);
        struct timeval start = current->exec_info.start;
        struct timeval end = current->exec_info.end;
        sprintf(tmp, "PID: %d\n", pid);
        write(open_res, &tmp, strlen(tmp));
        sprintf(tmp, "PROG_NAME: %s\n", prog_name);
        write(open_res, &tmp, strlen(tmp));
        sprintf(tmp, "Start Timeval: %ld\n", start);
        write(open_res, &tmp, strlen(tmp));
        sprintf(tmp, "End Timeval: %ld\n", end);
        write(open_res, &tmp, strlen(tmp));

        #pragma GCC diagnostic pop
        
        long end_time_ms = end.tv_sec * 1000 + end.tv_usec / 1000;
        long elapsed_seconds = end.tv_sec - start.tv_sec;
        long elapsed_useconds = end.tv_usec - start.tv_usec;
        long elapsed_time = (elapsed_seconds * 1000) + (elapsed_useconds / 1000);
        sprintf(tmp, "Total Execution time (ms): %ld\n", elapsed_time);
        write(open_res, &tmp, strlen(tmp));    
    }
}


void remove_exec(int pid, char* folder, struct timeval end, long elapsed){
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

    add_finExec(&current->exec_info, end, elapsed);
    add_to_file_finished(pid, folder);

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

//TODO to be removed!
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

int calculate_elapsed_time(struct timeval start, struct timeval end){
    long elapsed_seconds = end.tv_sec - start.tv_sec;
    long elapsed_useconds = end.tv_usec - start.tv_usec;
    long elapsed_time = (elapsed_seconds * 1000) + (elapsed_useconds / 1000);
    return elapsed_time;
}






//! STATS-TIME
int search_pid_finished(int* pids, int pids_size){
    int total_time = 0;
    int status;
    char outp[100];
    char file_in[sizeof(struct exec)];

    // Criar pipe para processo filho
    int pipes[2];
    if (pipe(pipes) == -1) {
        perror("Error creating pipe");
        return -1;
    }

    // Fazer tantos fork quanto os pids passados como argumento
    int elapsed_total=0;
    for (int i = 0; i < pids_size; i++) {
        int resf = fork();
        if (resf == -1) {
            sprintf(outp, "Error in fork #%d", i+1);
            perror(outp);
            return -1;
        }
        else if (resf == 0){
            close(pipes[0]);

            char folder_file[100];
            sprintf(folder_file, "%s%d.txt", fin_dir, pids[i]);
            int res_open = open(folder_file, O_RDONLY, 0660);
            if(res_open < 0){
                sprintf(outp, "Error opening file %d", pids[i]);
                perror(outp);
                _exit(-1);
            }

            struct exec file_exec;
            read(res_open, &file_exec, sizeof(struct exec));
            struct timeval start =  file_exec.start;
            struct timeval end =  file_exec.end;

            int elapsed_time = calculate_elapsed_time(start, end);
            write(pipes[1], &elapsed_time, sizeof(int));

            close(res_open);
            close(pipes[1]);
            _exit(0);

        } else {
            close(pipes[1]);
            int elapsed;
            read(pipes[0], &elapsed, sizeof(int));
            elapsed_total += elapsed;
            close(pipes[0]);
        }
    }

    // Retornar tempo total em ms
    return elapsed_total;
}

//! STATS-COMMAND
int search_pid_and_prog_finished(int* pids, int pids_size, char* command){
    int total_time = 0;
    int status;
    char outp[100];
    char file_in[sizeof(struct exec)];


    // Fazer tantos fork quanto os pids passados como argumento
    int num_times=0;
    for (int i = 0; i < pids_size; i++) {
        int resf = fork();
        if (resf == -1){
            sprintf(outp, "Error in fork #%d", i+1);
            perror(outp);
            return -1;
        } 
        else if(resf == 0){
            int ret;
            char folder_file[100];
            sprintf(folder_file, "%s%d.txt", fin_dir, pids[i]);
            int res_open = open(folder_file, O_RDONLY, 0660);
            if(res_open < 0){
                sprintf(outp, "Error opening file %d", pids[i]);
                perror(outp);
                _exit(0);
            }

            struct exec file_exec;
            if (read(res_open, &file_exec, sizeof(struct exec)) == -1) {
                sprintf(outp, "Error reading file %d", pids[i]);
                perror(outp);
                close(res_open);
                _exit(0);
            }            
            char prog_name[100];
            strcpy(prog_name, file_exec.prog_name);
            close(res_open);

            char* prog_name_noargs = strtok(prog_name, " ");

            sleep(1);
            int flag_equal = 1, ch, bigger_prog_name;
            if (strcmp(command, prog_name_noargs) != 0) flag_equal = 0;
            _exit(flag_equal);
            if(flag_equal == 0) _exit(0);
            else _exit(1);

        }
    }
    
    // Esperar que todos os processos filhos terminem
    for(int i = 0; i < pids_size; i++){
        //waitpid(resf, &status, 0);
        if (wait(&status) == -1) {
            sprintf(outp, "Error in waiting for child #%d", i+1);
            perror(outp);
            return -1;
        }
        printf("pai aux: %d\n", WEXITSTATUS(status));
        num_times += WEXITSTATUS(status);
        printf("num_times: %d\n", num_times);
    }

    // Retornar número de execusoes do command
    return num_times;
}


//! STATS-UNIQ
int search_uniq_finished(int *pids, int pids_size) { //concurrent
    int total_time = 0;
    int status;
    char outp[100];
    char file_in[sizeof(struct exec)];
    char uniq_progs[pids_size][100];

    int pipes[pids_size][2];

    // Criar pipes para cada processo filho
    for (int i = 0; i < pids_size; i++) {
        if (pipe(pipes[i]) == -1) {
            sprintf(outp, "Error creating pipe #%d", i+1);
            perror(outp);
            return -1;
        }
    }

    // Fazer tantos fork quanto os pids passados como argumento
    for (int i = 0; i < pids_size; i++) {
        int resf = fork();
        if (resf == -1) {
            sprintf(outp, "Error in fork #%d", i+1);
            perror(outp);
            return -1;
        } 
        
        else if (resf == 0) {
            // Filho vai escrever no pipe o nome do programa
            close(pipes[i][0]);
            char folder_file[100];
            sprintf(folder_file, "%s%d.txt", fin_dir, pids[i]);
            int res_open = open(folder_file, O_RDONLY, 0660);
            if (res_open < 0) {
                sprintf(outp, "Error opening file %d", pids[i]);
                perror(outp);
                int aux = 0;
                write(pipes[i][1], &aux, sizeof(int));
                close(pipes[i][1]);
                _exit(-1);
            }

            struct exec file_exec;
            read(res_open, &file_exec, sizeof(struct exec));
            char prog_name[100];
            strcpy(prog_name, file_exec.prog_name);

            close(res_open);

            int prog_name_size = strlen(prog_name);
            write(pipes[i][1], &prog_name_size, sizeof(int));
            write(pipes[i][1], prog_name, prog_name_size);

            close(pipes[i][1]);
            _exit(0);
        }
    }

    // Esperar que todos os processos filho terminem
    for (int i = 0; i < pids_size; i++) {
        if (wait(&status) == -1) {
            sprintf(outp, "Error in waiting for child #%d", i+1);
            perror(outp);
            return -1;
        }    
    }

    // Ler nomes dos programas dos pipes e guardar os únicos
    int uniq_size = 0;
    for (int i = 0; i < pids_size; i++) {
        close(pipes[i][1]);
        int prog_rec_size;
        char prog_received[100];
        if (read(pipes[i][0], &prog_rec_size, sizeof(int)) != 0) {
            read(pipes[i][0], prog_received, prog_rec_size);
            prog_received[prog_rec_size] = '\0'; // Making sure que a string é null terminated
            int idx;
            for (idx = 0; idx < uniq_size && strcmp(uniq_progs[idx], prog_received) != 0; idx++);
            if (idx == uniq_size) {
                strcpy(uniq_progs[idx], prog_received);
                uniq_size++;
            }
        }
        close(pipes[i][0]);
    }

    //DEBUG
    for (int i = 0; i < uniq_size; i++) printf("uniq[%d] %s\n", i, uniq_progs[i]);

    return 0;
}



int main(int argc, char* argv[]){
    char fifoname[50];
    int res_createfifo;
    int fd_clientServer;
    int res_readfifo;
    char fifo_in[100];
    char outp[200];
    llexec_size = 0;
    llfin_size = 0;

    if (argc != 2 && argc == 1) {
        sprintf(outp, "Finished executions folder name not provided\nPlease try again...\n");
        write(1, &outp, strlen(outp));
        exit(-1);
    }
    char* folder = argv[1];

    // Criar pipe para leitura do escrito pelos clientes
    sprintf(fifoname, "../tmp/%s", fifo_cliSer);
    res_createfifo = mkfifo(fifoname, 0660);
    if(res_createfifo == -1){
        if(errno != EEXIST) {
            perror("Error creating Client->Server pipe");
            exit(-1);
        }
    }

    // Inicializar as structs & guardar nome da pasta de execuções terminadas em var global
    currExecs = init_llexec();
    finExecs = init_llfin();
    sprintf(fin_dir, "../%s/", folder);

    // Ciclo infinito //TODO adicionar um signal?
    while(1){

        // Abrir o pipe criado
        fd_clientServer = open(fifoname, O_RDONLY);
        if(fd_clientServer == -1){
            perror("Error opening Client->Server pipe to read");
            exit(-1);
        }

        // Pipe opened
        int query_int;
        while((res_readfifo = read(fd_clientServer, &query_int, sizeof(int))) > 0){

            //! EXECUTE SINGLE
            if(query_int == 1){
                int store_request_id = request_id++;
                sprintf(outp, "[REQUEST #%d]  New execute single request\n", store_request_id);
                write(1, &outp, strlen(outp));

                //collect sent info from new request
                int pid;
                int prog_name_size;
                struct timeval start_time;
                read(fd_clientServer, &pid, sizeof(int));
                read(fd_clientServer, &prog_name_size, sizeof(int));
                char prog_name[prog_name_size+1];
                read(fd_clientServer, &prog_name, prog_name_size);
                prog_name[prog_name_size] = '\0'; //add null terminator
                read(fd_clientServer, &start_time, sizeof(struct timeval));
                
                long start_time_ms = start_time.tv_sec * 1000 + start_time.tv_usec / 1000;
                sprintf(outp, "[EXECUTE]     START: PID %d | Command \"%s\" | Start timeval %ld\n", pid, prog_name, start_time_ms);
                write(1, &outp, strlen(outp));

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

                sprintf(outp, "[EXECUTE]     END:   PID %d | End timeval %ld | Total time %ld ms\n", pid, end_time_ms, elapsed_time);
                write(1, &outp, strlen(outp));

                remove_exec(pid, folder, end_time, elapsed_time);
                close(fd_serverClient);


            

            //! EXECUTE PIPELINE
            } else if(query_int == 2) {
                int store_request_id = request_id++;
                sprintf(outp, "[REQUEST #%d]  New execute pipeline request\n", store_request_id);
                write(1, &outp, strlen(outp));

                //collect sent info from new request
                int pid;
                int prog_name_size;
                struct timeval start_time;
                read(fd_clientServer, &pid, sizeof(int));
                read(fd_clientServer, &prog_name_size, sizeof(int));
                char prog_name[prog_name_size+1];
                read(fd_clientServer, &prog_name, prog_name_size);
                prog_name[prog_name_size] = '\0'; //add null terminator
                read(fd_clientServer, &start_time, sizeof(struct timeval));
                
                long start_time_ms = start_time.tv_sec * 1000 + start_time.tv_usec / 1000;
                sprintf(outp, "[EXECUTE]     START: PID %d | Command \"%s\" | Start timeval %ld\n", pid, prog_name, start_time_ms);
                write(1, &outp, strlen(outp));

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

                sprintf(outp, "[EXECUTE]     END:   PID %d | End timeval %ld | Total time %ld ms\n", pid, end_time_ms, elapsed_time);
                write(1, &outp, strlen(outp));

                remove_exec(pid, folder, end_time, elapsed_time);
                close(fd_serverClient);


            //! STATUS
            } else if(query_int == 3) {
                int store_request_id = request_id++;
                sprintf(outp, "[REQUEST #%d]  New status request\n", store_request_id);
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

                //verificar se há algum programa em execução
                int sizell = size_llexec();
                write(fd_serverClient, &sizell, sizeof(int));
                if(sizell == 0){
                    sprintf(outp, "[STATUS]      That aren't any programs currently running\n");
                    write(1, &outp, strlen(outp));
                } else {
                    //llexec* tmp = currExecs;
                    for(llexec* tmp = currExecs; tmp != NULL; tmp=tmp->next){
                        struct timeval til_now;
                        gettimeofday(&til_now, NULL);
                        struct timeval start = tmp->exec_info.start;

                        long elapsed_seconds = til_now.tv_sec - start.tv_sec;
                        long elapsed_useconds = til_now.tv_usec - start.tv_usec;
                        long elapsed_time = (elapsed_seconds * 1000) + (elapsed_useconds / 1000);   

                        sprintf(outp, "%d   %s   %ld ms\n", tmp->exec_info.pid, tmp->exec_info.prog_name, elapsed_time);
                        int exec_size = strlen(outp);
                        write(fd_serverClient, &exec_size, sizeof(int));
                        write(fd_serverClient, &outp, exec_size);
                    }
                }

                print_llexec(); //debug
                print_llfin();  //debug


            //! STATS-TIME
            } else if(query_int == 4) {
                int store_request_id = request_id++;
                sprintf(outp, "[REQUEST #%d]  New stats-time request\n", store_request_id);
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
                //int sizell = size_llfin();
                //write(fd_serverClient, &sizell, sizeof(int));
                //if(sizell == 0){
                //    sprintf(outp, "[STATS_TIME]  That aren't any programs that have already finished\n");
                //    write(1, &outp, strlen(outp));
                //} else {
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
                int total_time = search_pid_finished(pids, pids_size);

                //enviar string de output
                sprintf(outp, "Total execution time is %d ms\n", total_time);
                int tot_size = strlen(outp);
                write(fd_serverClient, &tot_size, sizeof(int));
                write(fd_serverClient, &outp, tot_size);
                //}

                sprintf(outp, "[REQUEST #%d]  Ended stats-time\n", store_request_id);
                write(1, &outp, strlen(outp));


            //! STATS-COMMAND
            } else if(query_int == 5) {
                int store_request_id = request_id++;
                sprintf(outp, "[REQUEST #%d]  New stats-command request\n", store_request_id);
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

                //ler nome do programa
                int command_size;
                read(fd_clientServer, &command_size, sizeof(int));
                char command[command_size];
                read(fd_clientServer, &command, command_size+1);
                
                //ler pids e pids_size
                int pids_size;
                read(fd_clientServer, &pids_size, sizeof(int));
                int pids[pids_size];
                for(int i = 0; i < pids_size; i++){
                    int tmp;
                    read(fd_clientServer, &tmp, sizeof(int));
                    pids[i] = tmp;
                }
                
                //calcular tempo total
                int total_execs_prog = search_pid_and_prog_finished(pids, pids_size, command);

                //enviar string de output
                sprintf(outp, "Prog was executed %d times\n", total_execs_prog);
                int tot_size = strlen(outp);
                write(fd_serverClient, &tot_size, sizeof(int));
                write(fd_serverClient, &outp, tot_size);
                
                sprintf(outp, "[REQUEST #%d]  Ended stats-command\n", store_request_id);
                write(1, &outp, strlen(outp));


            //! STATS-UNIQ
            } else if(query_int == 6) {
                int store_request_id = request_id++;
                sprintf(outp, "[REQUEST #%d]  New stats-uniq request\n", store_request_id);
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
                //int sizell = size_llfin();
                //write(fd_serverClient, &sizell, sizeof(int));
                //if(sizell == 0){
                //    sprintf(outp, "[STATS_TIME]  That aren't any programs that have already finished\n");
                //    write(1, &outp, strlen(outp));
                //} else {
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
                
                int total_time = search_uniq_finished(pids, pids_size);

                //enviar string de output
                sprintf(outp, "Uniq Progs Number: %d\n", total_time);
                int tot_size = strlen(outp);
                write(fd_serverClient, &tot_size, sizeof(int));
                write(fd_serverClient, &outp, tot_size);
                //}

                sprintf(outp, "[REQUEST #%d]  Ended stats-uniq\n", store_request_id);
                write(1, &outp, strlen(outp));
                return 0;
            }
        }
    }
    
    //close(fd_clientServer);
    unlink(fifoname);
    return 0;
}