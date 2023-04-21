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
#define fin_dir "../finished/"

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


int search_pid_and_prog_finished(int* pids, int pids_size, char* command){
    int total_time = 0;
    int status;
    char outp[100];
    char file_in[sizeof(struct exec)];

    //criar os varios pipes
    int pipes[2];
    if(pipe(pipes) < 0){
        perror("Error creating pipes");
        exit(1);
    }
    int back = dup(1);

    int num_times=0;
    for(int i = 0; i < pids_size; i++){
        int resf = fork();
        if(resf == 0){
            close(pipes[0]);

            int ret;
            char folder_file[100];
            sprintf(folder_file, "%s%d.txt", fin_dir, pids[i]);
            int res_open = open(folder_file, O_RDONLY, 0660);
            if(res_open < 0){
                sprintf(outp, "Error opening file %d", pids[i]);
                perror(outp);
                return 0;
                _exit(-1);
            }

            struct exec file_exec;
            read(res_open, &file_exec, sizeof(struct exec));
            char prog_name[100];
            strcpy(prog_name, file_exec.prog_name);
            //if (strstr(string1, string2) != NULL) tambem podiamos usar isto
            //mmas ao testar char a char comparamos logo a partir do primeiro

            printf("prog_name read: %c\n", prog_name[0]);
            //testar o write com o backup feito em cima
            printf("command: %c\n", command[0]);

            int flag_equal = 1;
            for(int ch = 0; ch < strlen(command) && flag_equal; ch++){
                if(command[ch] == prog_name[ch]) flag_equal = 0;
            }
            if(flag_equal) ret = 0;
            else ret = 1; //significa que command é subset de prog_name
            write(pipes[1], &ret, sizeof(int));

            close(res_open);
            close(pipes[1]);
            _exit(0);
        } else {
            close(pipes[1]);
            int ret;
            read(pipes[0], &ret, sizeof(int));
            num_times += ret;
            close(pipes[0]);
        }
    }

    return num_times;
}


int search_pid_finished(int* pids, int pids_size){

    int total_time = 0;
    int status;
    char outp[100];
    char file_in[sizeof(struct exec)];

    //criar os varios pipes
    int pipes[2];
    if(pipe(pipes) < 0){
        perror("Error creating pipes");
        exit(1);
    }

    int elapsed_total=0;
    for(int i = 0; i < pids_size; i++){
        int resf = fork();
        if(resf == 0){
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

    return elapsed_total;
}

/* int search_pid_finishedExecs(int* pids, int pids_size){

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
} */


int main(int argc, char* argv[]){
    char fifoname[50];
    int res_createfifo;
    int fd_clientServer;
    int res_readfifo;
    char fifo_in[100];
    char outp[200];
    llexec_size = 0;
    llfin_size = 0;

    if(argc != 2){
        sprintf(outp, "Finished executions folder name not provided or not the correct number of arguments\nPlease try again...\n");
        write(1, &outp, strlen(outp));
        exit(-1);
    }
    char* folder = argv[1];

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


            } else if(query_int == 30) { //status
                int store_request_id = request_id++;
                sprintf(outp, "[REQUEST #%d] New status request\n", store_request_id);
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





            } else if(query_int == 40) { //stats-time
                int store_request_id = request_id++;
                sprintf(outp, "[REQUEST #%d] New stats-time request\n", store_request_id);
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

            } else if(query_int == 50) { //stats-command
                int store_request_id = request_id++;
                sprintf(outp, "[REQUEST #%d] New stats-command request\n", store_request_id);
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
                read(fd_clientServer, &command, command_size);

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

            } else if(query_int == 60) { //stats_uniq

                return 0;
            }
        }
    }
    //close(fd_clientServer);
    unlink(fifoname);
    return 0;
}