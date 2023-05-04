#include "common.h"

int fd_serCli;
int llexec_size;
int llfin_size;
int request_id = 0;
char fin_dir[50];


// Lista ligada com informações das execuções atuais
typedef struct llexec {
    exec exec_info;
    struct llexec* next;
} llexec;

llexec* currExecs;



// Funções auxiliares para as listas ligadas
llexec* init_llexec() {
    llexec* head = NULL;
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


void add_to_file_finished(llexec* finExec, char* folder){

    char folder_file[100];
    sprintf(folder_file, "%s%d.txt", fin_dir, finExec->exec_info.pid);
    int open_res = open(folder_file, O_CREAT | O_TRUNC | O_WRONLY, 0660);
    if (open_res < 0){
        perror("Error opening file that stores information of finished execution");
        exit(-1);
    }

    // Escrevemos a struct e depois cada valor formatado (so that's readable by struct and by text)
    write(open_res, &finExec->exec_info, sizeof(struct exec));
    char tmp[100];
    sprintf(tmp, "\n\n");
    write(open_res, &tmp, strlen(tmp));


    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wformat"
    
    int pid = finExec->exec_info.pid;
    int prog_name_size = strlen(finExec->exec_info.prog_name);
    char prog_name[prog_name_size];
    sprintf(prog_name, "%s", finExec->exec_info.prog_name);
    struct timeval start = finExec->exec_info.start;
    struct timeval end = finExec->exec_info.end;

    sprintf(tmp, "Process ID: %d\n", pid);
    write(open_res, &tmp, strlen(tmp));
    sprintf(tmp, "Program: %s\n", prog_name);
    write(open_res, &tmp, strlen(tmp));
    sprintf(tmp, "Start Time: %ld.%06ld\n", start.tv_sec, start.tv_usec);
    write(open_res, &tmp, strlen(tmp));
    sprintf(tmp, "End Time: %ld.%06ld\n", end.tv_sec, end.tv_usec);
    write(open_res, &tmp, strlen(tmp));

    #pragma GCC diagnostic pop

    int elapsed_time = calculate_elapsed_time(start, end);
    sprintf(tmp, "Total Execution Time: %d ms\n", elapsed_time);
    write(open_res, &tmp, strlen(tmp));

}

void remove_exec(int pid, char* folder, struct timeval end, long elapsed){
    llexec* current = currExecs;
    llexec* previous = NULL;

    while (current != NULL && current->exec_info.pid != pid) {
        previous = current;
        current = current->next;
    }
    
    if (current == NULL)
        exit(-1);

    if (previous == NULL)
        currExecs = current->next;
    else
        previous->next = current->next;

    add_to_file_finished(current, folder);
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

int size_llexec(){
    int i;
    llexec* current = currExecs;
    for(i = 0; current != NULL; i++, current=current->next);
    return i;
}







//! STATS-TIME
int search_time_finished(int* pids, int pids_size){
    int total_time = 0;
    int status;
    char outp[100];
    char file_in[sizeof(struct exec)];

    // Criar pipe para processo filho
    int p[2];
    if (pipe(p) == -1) {
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
            close(p[0]);

            char folder_file[100];
            sprintf(folder_file, "%s%d.txt", fin_dir, pids[i]);
            int res_open = open(folder_file, O_RDONLY, 0660);
            if(res_open < 0){
                sprintf(outp, "Error opening file %d", pids[i]);
                perror(outp);
                int error_add_zero = 0;
                write(p[1], &error_add_zero, sizeof(int));
                close(p[1]);
                _exit(-1);
            }

            struct exec file_exec;
            read(res_open, &file_exec, sizeof(struct exec));
            struct timeval start =  file_exec.start;
            struct timeval end =  file_exec.end;

            int elapsed_time = calculate_elapsed_time(start, end);
            write(p[1], &elapsed_time, sizeof(int));

            close(res_open);
            close(p[1]);
            _exit(0);

        }
    }

    // Esperar que todos os processos filho terminem
    for (int i = 0; i < pids_size; i++) {
        if (wait(&status) == -1) {
            sprintf(outp, "Error in waiting for child process");
            perror(outp);
            return -1;
        }
        close(p[1]);
        int elapsed;
        read(p[0], &elapsed, sizeof(int));
        elapsed_total += elapsed;
    }
    close(p[0]);

    // Retornar tempo total em ms
    return elapsed_total;
}

//! STATS-COMMAND
int search_command_finished(int* pids, int pids_size, char* command){
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

            int flag_equal = 1, ch, bigger_prog_name;
            if (strcmp(command, prog_name_noargs) != 0) flag_equal = 0;
            _exit(flag_equal);
            if(flag_equal == 0) _exit(0);
            else _exit(1);
        }
    }
    
    // Esperar que todos os processos filho terminem
    for(int i = 0; i < pids_size; i++){
        //waitpid(resf, &status, 0);
        if (wait(&status) == -1) {
            sprintf(outp, "Error in waiting for child process");
            perror(outp);
            return -1;
        }
        printf("pai aux: %d\n", WEXITSTATUS(status));
        num_times += WEXITSTATUS(status);
        printf("num_times: %d\n", num_times);
    }

    // Retornar número de execuções do command
    return num_times;
}


//! STATS-UNIQ
int search_uniq_finished(int *pids, int pids_size, int fd_serCli) {
    int total_time = 0;
    int status;
    char outp[100];
    char file_in[sizeof(struct exec)];
    char uniq_progs[pids_size][90];

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
                int aux = 0;
                write(pipes[i][1], &aux, sizeof(int));
                close(pipes[i][1]);
                perror(outp);
                _exit(-1);
            }

            struct exec file_exec;
            read(res_open, &file_exec, sizeof(struct exec));
            char prog_name[100];
            strcpy(prog_name, file_exec.prog_name);

            close(res_open);

            int prog_name_size = strlen(prog_name);
            write(pipes[i][1], &prog_name_size, sizeof(int));
            write(pipes[i][1], prog_name, prog_name_size + 1);

            close(pipes[i][1]);
            _exit(0);
        }
    }

    // Esperar que todos os processos filho terminem
    for (int i = 0; i < pids_size; i++) {
        if (wait(&status) == -1) {
            sprintf(outp, "Error in waiting for child process");
            perror(outp);
            return -1;
        }
    }

    // Ler nomes dos programas dos pipes e guardar os únicos
    int uniq_size = 0;
    for (int i = 0; i < pids_size; i++) {
        close(pipes[i][1]);
        int prog_rec_size;
        read(pipes[i][0], &prog_rec_size, sizeof(int));
        if (prog_rec_size != 0) {
            char prog_received[prog_rec_size+1];
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
    //for (int i = 0; i < uniq_size; i++) printf("%s\n", uniq_progs[i]);

    write(fd_serCli, &uniq_size, sizeof(int));
    for(int i = 0; i < uniq_size; i++){
        sprintf(outp, "%s\n", uniq_progs[i]);
        int outp_size = strlen(outp);
        write(1, outp, outp_size);
        write(fd_serCli, &outp_size, sizeof(int));
        write(fd_serCli, outp, outp_size);
    }

    // TODO Enviar nomes dos programas únicos executados pelos pids argumento
    //enviar string de output
    //sprintf(outp, "Uniq Progs Number: %d\n", uniq_size);
    //int tot_size = strlen(outp);
    //write(fd_serCli, &tot_size, sizeof(int));
    //write(fd_serCli, &outp, tot_size);

    return uniq_size;
}




#include <stdio.h>

void print_exec(struct exec e) {
    printf("query_int: %d\n", e.query_int);
    printf("pid_pai: %d\n", e.pid_pai);
    printf("pid: %d\n", e.pid);
    printf("prog_name: %s\n", e.prog_name);
    printf("start: %ld.%06ld\n", e.start.tv_sec, e.start.tv_usec);
    printf("end: %ld.%06ld\n", e.end.tv_sec, e.end.tv_usec);
    printf("pids_search: ");
    for (int i = 0; i < e.pids_search_size; i++) {
        printf("%d ", e.pids_search[i]);
    }
    printf("\npids_search_size: %d\n", e.pids_search_size);
}






//! MAIN
int main(int argc, char* argv[]){
    char fifo_cliSer_name[50];
    int res_createfifo, fd_cliSer, res_readfifo;
    char outp[200];
    llexec_size = 0;
    llfin_size = 0;

    // Verificar se o nome da pasta de execuções terminadas foi fornecido como argumento
    if (argc == 1) {
        sprintf(outp, "Finished executions folder name not provided\nPlease try again...\n");
        write(1, &outp, strlen(outp));
        exit(-1);
    }

    // Criar pipe para leitura do escrito pelos clientes
    sprintf(fifo_cliSer_name, "../tmp/%s", fifo_cliSer);
    res_createfifo = mkfifo(fifo_cliSer_name, 0660);
    if(res_createfifo == -1){
        if(errno != EEXIST) {
            perror("Error creating Client->Server pipe");
            exit(-1);
        }
    }

    // Inicializar as structs & guardar nome da pasta de execuções terminadas em var global
    currExecs = init_llexec();

    // Guardar nome da pasta com as execuções terminadas num var global
    char* folder = argv[1];
    sprintf(fin_dir, "../%s/", folder);

    // Criação da pasta para as execuções terminadas (caso ainda não exista)
    int mkdir_ret = mkdir(fin_dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (mkdir_ret == 0) {
        sprintf(outp, "Folder %s created successfully\n", fin_dir);
        write(1, &outp, strlen(outp));
    } else if (errno == EEXIST) {
        sprintf(outp, "Folder %s already exists\n", fin_dir);
        write(1, &outp, strlen(outp));
    } else {
        sprintf(outp, "Error creating folder %s for finished executions", fin_dir);
        perror(outp);
    }

    // Ciclo infinito para ler pedidos dos clientes
    while(1){

        // Abrir o pipe criado
        fd_cliSer = open(fifo_cliSer_name, O_RDONLY);
        if(fd_cliSer == -1) {
            perror("Error opening Client->Server pipe to read");
            exit(-1);
        }
        
        // Pipe opened
        // Ler pedidos dos clientes de tamanho fixo sizeof(struct exec)
        //* VERIFICAR QUE NAO É MAIOR QUE PIPE_BUF
        struct exec query;
        while((res_readfifo = read(fd_cliSer, &query, sizeof(struct exec))) > 0){

            print_exec(query);
            printf("PIPE_BUF: %d\n", PIPE_BUF);
            printf("sixeof(struct): %ld\n", sizeof(struct exec));

            //! EXECUTE SINGLE or EXECUTE PIPELINE
            if(query.query_int == 1 || query.query_int == 2 || query.query_int == 3){ 
                int store_request_id = request_id++;
                if(query.query_int == 1 || query.query_int == 2){
                    sprintf(outp, "[REQUEST #%d]  New execute request\n", store_request_id);
                    write(1, &outp, strlen(outp));

                    //collect sent info from new request
                    int pid = query.pid;
                    char prog_name[100];
                    strcpy(prog_name, query.prog_name);
                    struct timeval start = query.start;
                    //prog_name[prog_name_size] = '\0'; //add null terminator
                    
                    
                    long start_ms = start.tv_sec * 1000 + start.tv_usec / 1000;
                    sprintf(outp, "[EXECUTE] START      PID %d ║ Command \"%s\" ║ Start timeval %ld\n", pid, prog_name, start_ms);
                    write(1, &outp, strlen(outp));

                    //exec* prog_exec = newExec(pid, prog_name, start);
                    add_exec(pid, prog_name, start);
                    print_llexec();
                }

                //! EXECUTE END
                else if (query.query_int == 3) {
                    // Guardar informação do pedido necessário
                    int pid = query.pid;
                    char prog_name[100];
                    strcpy(prog_name, query.prog_name);
                    struct timeval end = query.end;
                    struct timeval start = query.start;

                    // verificar se há programas a executar que possam ser terminados (algo desnecessário)
                    int sizell = size_llexec();
                    write(fd_serCli, &sizell, sizeof(int));
                    if(sizell == 0){
                        sprintf(outp, "[EXECUTE] END        That aren't any programs currently running\n");
                        write(1, &outp, strlen(outp));
                        exit(0);
                    }

                    // Calcular tempo de execução em ms
                    long end_ms = end.tv_sec * 1000 + end.tv_usec / 1000;
                    long elapsed_time = calculate_elapsed_time(start, end);
                    sprintf(outp, "[EXECUTE] END        PID %d ║ End timeval %ld ║ Total time %ld ms\n", pid, end_ms, elapsed_time);
                    write(1, &outp, strlen(outp));

                    // Remover execução da lista ligada de execuções ativas
                    remove_exec(pid, folder, end, elapsed_time);
                    
                    sprintf(outp, "[REQUEST #%d]  Ended execute request\n", store_request_id);
                    write(1, &outp, strlen(outp));
                }
            }

            else {
                // Abrir o fifo de comunicação Servidor->Cliente identificado pelo pid do pai
                int pid_pai = query.pid_pai;
                char fifo_serCli_name[strlen(fifo_serCli)+sizeof(int)];
                sprintf(fifo_serCli_name, "../tmp/%s_%d", fifo_serCli, pid_pai);               

                fd_serCli = open(fifo_serCli_name, O_WRONLY);
                if(fd_serCli == -1){
                    perror("Error opening Server->Client pipe to write");
                    exit(-1);
                }

                // Criar processo filho para tratar de pedidos de status/stats-x
                int resf = fork();
                if (resf == -1) {
                    perror("Error in fork to handle status/stats-x request");
                    return -1;
                }
                else if (resf == 0) {

                    //! STATUS
                    if (query.query_int == 4) {
                        int store_request_id = request_id;
                        sprintf(outp, "[REQUEST #%d]  New status request\n", store_request_id);
                        write(1, &outp, strlen(outp));

                        int resf = fork();
                        if (resf == -1) {
                            perror("Error creating child process to handle Status request");
                            exit(-1);
                        }
                        else if (resf == 0) {

                            // verificar se há algum programa em execução
                            int sizell = size_llexec();
                            write(fd_serCli, &sizell, sizeof(int));
                            if (sizell == 0) {
                                sprintf(outp, "[STATUS]      That aren't any programs currently running\n");
                                write(1, &outp, strlen(outp));
                                exit(0); // TODO fecha o processo child?
                            }

                            for (llexec *tmp = currExecs; tmp != NULL; tmp = tmp->next) {
                                struct timeval til_now;
                                gettimeofday(&til_now, NULL);
                                struct timeval start = tmp->exec_info.start;

                                long elapsed_time = calculate_elapsed_time(start, til_now);
                                sprintf(outp, "%d   %s   %ld ms\n", tmp->exec_info.pid, tmp->exec_info.prog_name, elapsed_time);
                                int exec_size = strlen(outp);
                                write(fd_serCli, &exec_size, sizeof(int));
                                write(fd_serCli, &outp, exec_size);
                            }
                            // print_llexec(); //debug
                            // print_llfin();  //debug
                            _exit(0);
                        }

                        sprintf(outp, "[REQUEST #%d]  Ended status request\n", store_request_id);
                        write(1, &outp, strlen(outp));
                    }


                    //! STATS-TIME
                    else if (query.query_int == 5) {
                        int store_request_id = request_id++;
                        sprintf(outp, "[REQUEST #%d]  New stats-time request\n", store_request_id);
                        write(1, &outp, strlen(outp));

                        // ler pids e pids_size
                        int pids_search_size = query.pids_search_size;
                        int pids_search[100];

                        for (int i = 0; i < pids_search_size; i++) {
                            pids_search[i] = query.pids_search[i];
                            printf("pids_search[%d]: %d\n", i, pids_search[i]);
                        }

                        // calcular tempo total
                        int total_time = search_time_finished(pids_search, pids_search_size);

                        // enviar string de output
                        sprintf(outp, "Total execution time is %d ms\n", total_time);
                        int tot_size = strlen(outp);
                        write(fd_serCli, &tot_size, sizeof(int));
                        write(fd_serCli, &outp, tot_size);
                        write(1, &outp, tot_size);

                        sprintf(outp, "[REQUEST #%d]  Ended stats-time request\n", store_request_id);
                        write(1, &outp, strlen(outp));
                    }


                    //! STATS-COMMAND
                    else if (query.query_int == 6) {
                        int store_request_id = request_id++;
                        sprintf(outp, "[REQUEST #%d]  New stats-command request\n", store_request_id);
                        write(1, &outp, strlen(outp));

                        // ler nome do programa
                        char prog_name[100];
                        strcpy(prog_name, query.prog_name);

                        // ler pids e pids_size
                        int pids_search_size = query.pids_search_size;
                        int pids_search[100];

                        for (int i = 0; i < pids_search_size; i++)
                        {
                            pids_search[i] = query.pids_search[i];
                            printf("pids_search[%d]: %d\n", i, pids_search[i]);
                        }

                        // calcular numero de vezes total
                        int total_execs_prog = search_command_finished(pids_search, pids_search_size, prog_name);

                        // enviar string de output
                        sprintf(outp, "%s was executed %d times\n", prog_name, total_execs_prog);
                        int tot_size = strlen(outp);
                        write(fd_serCli, &tot_size, sizeof(int));
                        write(fd_serCli, &outp, tot_size);

                        sprintf(outp, "[REQUEST #%d]  Ended stats-command request\n", store_request_id);
                        write(1, &outp, strlen(outp));
                    }


                    //! STATS-UNIQ
                    else if (query.query_int == 7) {
                        int store_request_id = request_id++;
                        sprintf(outp, "[REQUEST #%d]  New stats-uniq request\n", store_request_id);
                        write(1, &outp, strlen(outp));

                        // ler pids e pids_size
                        int pids_search_size = query.pids_search_size;
                        int pids_search[100];

                        for (int i = 0; i < pids_search_size; i++) {
                            pids_search[i] = query.pids_search[i];
                            printf("pids_search[%d]: %d\n", i, pids_search[i]);
                        }

                        search_uniq_finished(pids_search, pids_search_size, fd_serCli);

                        sprintf(outp, "[REQUEST #%d]  Ended stats-uniq request\n", store_request_id);
                        write(1, &outp, strlen(outp));
                    }
                    _exit(0);
                }
                unlink(fifo_serCli_name);
                request_id++;
                close(fd_serCli);
            }
        }
    }
    
    close(fd_cliSer);
    unlink(fifo_cliSer_name);
    return 0;
}