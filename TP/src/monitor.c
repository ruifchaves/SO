#include <limits.h>    //pipe_buf
#include "common.h"
#define P_BUF PIPE_BUF

int fd_serCli;
int request_id = 0;
char fin_dir[50];


// Lista ligada com informações das execuções atuais
typedef struct llexec {
    exec exec_info;
    struct llexec* next;
} llexec;

llexec* currExecs;


// Função que inicializa a lista ligada
llexec* init_llexec() {
    llexec* head = NULL;
    return head;
}

// Função que adiciona uma nova execução à lista ligada
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


// Função que guarda uma execução acabada de terminar num ficheiro numa pasta especificada
void add_to_file_finished(llexec* finExec, struct timeval end){

    char folder_file[100];
    sprintf(folder_file, "%s%d.txt", fin_dir, finExec->exec_info.pid);
    int open_res = open(folder_file, O_CREAT | O_TRUNC | O_WRONLY, 0660);
    if (open_res < 0){
        perror("Error opening file that stores information of finished execution");
        exit(-1);
    }

    // Escrevemos a struct e depois cada valor formatado (so that's readable by struct and by text)
    finExec->exec_info.end = end;
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
    char readable_start_time[30];
    char readable_end_time[30];

    sprintf(tmp, "Process ID:           %d\n", pid);
    write(open_res, &tmp, strlen(tmp));
    sprintf(tmp, "Program:              %s\n", prog_name);
    write(open_res, &tmp, strlen(tmp));

    // Format the start time
    strftime(readable_start_time, sizeof(readable_start_time), "%Y-%m-%d %H:%M:%S", localtime(&start.tv_sec));
    sprintf(tmp, "Start Time:           %s.%03ld\n", readable_start_time, start.tv_usec / 1000);
    write(open_res, &tmp, strlen(tmp));

    // Format the end time
    strftime(readable_end_time, sizeof(readable_end_time), "%Y-%m-%d %H:%M:%S", localtime(&end.tv_sec));
    sprintf(tmp, "End Time:             %s.%03ld\n", readable_end_time, end.tv_usec / 1000);
    write(open_res, &tmp, strlen(tmp));
    #pragma GCC diagnostic pop

    int elapsed_time = calculate_elapsed_time(start, end);
    sprintf(tmp, "Total Execution Time: %d ms\n", elapsed_time);
    write(open_res, &tmp, strlen(tmp));

}

// Função que remove uma execução da lista ligada, uitlizada quando uma execução termina
void remove_exec(int pid, struct timeval end){
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

    add_to_file_finished(current, end);
    free(current);
}

// Função não utilizada, mas que pode ser útil para debug
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

// Função que verifica se há execuções a decorrer atualmente
int size_llexec(){
    int i;
    llexec* current = currExecs;
    for(i = 0; current != NULL; i++, current=current->next);
    return i;
}

// Função que imprime uma query recebida devidamente formatada
void print_exec(struct exec e) {
    char output[500];

    if(e.query_int == 1 || e.query_int == 2 || e.query_int == 3){
        sprintf(output, "[NEW REQUEST] Request ID: %d ║ PID: %d ║ PID: %d ║ Comando: %s ║ %ld.%06ld ║ %ld.%06ld\n",
                        e.query_int, e.pid_pai, e.pid, e.prog_name, e.start.tv_sec, e.start.tv_usec, e.end.tv_sec, e.end.tv_usec);    
    } 
    else {
        if(e.query_int == 4){
            sprintf(output, "[NEW REQUEST] Request ID: %d ║ PID: %d ║ PID: %d\n", e.query_int, e.pid_pai, e.pid);
        }
        else {
            if(e.query_int == 6)
                sprintf(output, "[NEW REQUEST] Request ID: %d ║ PID: %d ║ PID: %d ║ Comando: %s ║ ", e.query_int, e.pid_pai, e.pid, e.prog_name);
            else 
                sprintf(output, "[NEW REQUEST] Request ID: %d ║ PID: %d ║ PID: %d ║ ", e.query_int, e.pid_pai, e.pid);

            char pid_search_str[100]; // Adjust the size as needed
            int pid_search_length = sprintf(pid_search_str, "#PIDs: %d ║ ", e.pids_search_size);

            for (int i = 0; i < e.pids_search_size; i++) {
                pid_search_length += sprintf(pid_search_str + pid_search_length, "%d; ", e.pids_search[i]);
            }

            strcat(output, pid_search_str);
            strcat(output, "\n");
        }
    }

    write(1, output, strlen(output));
}





//! STATS-TIME
int search_time_finished(int* pids, int pids_size, int request_id){
    int total_time = 0;
    int status;
    char outp[100];
    char file_in[sizeof(struct exec)];

    // Criar pipe para processo filho
    int p[2];
    if (pipe(p) == -1) {
        sprintf(outp, "[REQUEST #%d]  Error creating pipe", request_id);
        perror(outp);
        return -1;
    }

    // Fazer tantos fork quanto os pids passados como argumento
    int elapsed_total=0;
    for (int i = 0; i < pids_size; i++) {
        int resf = fork();
        if (resf == -1) {
            sprintf(outp, "[REQUEST #%d]  Error in fork #%d", request_id, i+1);
            perror(outp);
            return -1;
        }
        else if (resf == 0){
            close(p[0]);

            char folder_file[100];
            sprintf(folder_file, "%s%d.txt", fin_dir, pids[i]);
            int res_open = open(folder_file, O_RDONLY, 0660);
            if(res_open < 0){
                sprintf(outp, "[REQUEST #%d]  Error opening file %d", request_id, pids[i]);
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
            sprintf(outp, "[REQUEST #%d]  Error in waiting for child process", request_id);
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
int search_command_finished(int* pids, int pids_size, char* command, int request_id){
    int total_time = 0;
    int status;
    char outp[100];
    char file_in[sizeof(struct exec)];

    // Criar pipe para processo filho
    int pipes[2];
    if (pipe(pipes) == -1) {
        sprintf(outp, "[REQUEST #%d]  Error creating pipe", request_id);
        perror(outp);
        return -1;
    }

    // Fazer tantos fork quanto os pids passados como argumento
    int num_times=0;
    for (int i = 0; i < pids_size; i++) {
        int resf = fork();
        if (resf == -1){
            sprintf(outp, "[REQUEST #%d]  Error in fork #%d", request_id, i+1);
            perror(outp);
            return -1;
        } 
        else if(resf == 0){
            int ret;
            char folder_file[100];
            sprintf(folder_file, "%s%d.txt", fin_dir, pids[i]);
            int res_open = open(folder_file, O_RDONLY, 0660);
            if(res_open < 0){
                sprintf(outp, "[REQUEST #%d]  Error opening file %d", request_id, pids[i]);
                perror(outp);
                _exit(0);
            }

            struct exec file_exec;
            if (read(res_open, &file_exec, sizeof(struct exec)) == -1) {
                sprintf(outp, "[REQUEST #%d]  Error reading file %d", request_id, pids[i]);
                perror(outp);
                close(res_open);
                _exit(0);
            }            
            char prog_name[100];
            strcpy(prog_name, file_exec.prog_name);
            close(res_open);

            int is_equal = 0; //1 igual, 0 diferente
            if (strchr(command, ' ') == NULL) {  //se nao tem espaco (só "sleep") compara com o nome do prog truncado
                char* prog_name_noargs = strtok(prog_name, " ");
                if (strcmp(command, prog_name_noargs) == 0) is_equal = 1;
            } 
            else //senao compara com o nome todo
                if (strcmp(command, prog_name) == 0) is_equal = 1;
            
            _exit(is_equal);
        }
    }
    
    // Esperar que todos os processos filho terminem
    for(int i = 0; i < pids_size; i++){
        //waitpid(resf, &status, 0);
        if (wait(&status) == -1) {
            sprintf(outp, "[REQUEST #%d]  Error in waiting for child process", request_id);
            perror(outp);
            return -1;
        }
        num_times += WEXITSTATUS(status);
    }

    // Retornar número de execuções do command
    return num_times;
}


//! STATS-UNIQ
int search_uniq_finished(int *pids, int pids_size, int fd_serCli, int request_id) {
    int total_time = 0;
    int status;
    char outp[100];
    char file_in[sizeof(struct exec)];
    char uniq_progs[pids_size][90];

    int pipes[pids_size][2];

    // Criar pipes para cada processo filho
    for (int i = 0; i < pids_size; i++) {
        if (pipe(pipes[i]) == -1) {
            sprintf(outp, "[REQUEST #%d]  Error creating pipe #%d", request_id, i+1);
            perror(outp);
            return -1;
        }
    }

    // Fazer tantos fork quanto os pids passados como argumento
    for (int i = 0; i < pids_size; i++) {
        int resf = fork();
        if (resf == -1) {
            sprintf(outp, "[REQUEST #%d]  Error in fork #%d", request_id, i+1);
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
                sprintf(outp, "[REQUEST #%d]  Error opening file %d", request_id, pids[i]);
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
            sprintf(outp, "[REQUEST #%d]  Error in waiting for child process", request_id);
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

    // Informar o Cliente de cada programa único encontrado
    write(fd_serCli, &uniq_size, sizeof(int));
    for(int i = 0; i < uniq_size; i++){
        sprintf(outp, "%s\n", uniq_progs[i]);
        int outp_size = strlen(outp);
        write(1, outp, outp_size);
        write(fd_serCli, &outp_size, sizeof(int));
        write(fd_serCli, outp, outp_size);
    }

    return uniq_size;
}


//! MAIN
int main(int argc, char* argv[]){
    char fifo_cliSer_name[50];
    int res_createfifo, fd_cliSer, res_readfifo;
    char outp[200];

    // Verificar se o nome da pasta de execuções terminadas foi fornecido como argumento
    if (argc == 1) {
        sprintf(outp, "[STARTING]  Finished executions folder name not provided\nPlease try again...\n");
        write(1, &outp, strlen(outp));
        exit(-1);
    }

    // Criar pipe para leitura do escrito pelos clientes
    sprintf(fifo_cliSer_name, "../tmp/%s", fifo_cliSer);
    res_createfifo = mkfifo(fifo_cliSer_name, 0660);
    if(res_createfifo == -1){
        if(errno != EEXIST) {
            perror("[STARTING]  Error creating Client->Server pipe");
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
        sprintf(outp, "[STARTING]  Folder %s created successfully\n", fin_dir);
        write(1, &outp, strlen(outp));
    } else if (errno == EEXIST) {
        sprintf(outp, "[STARTING]  Folder %s already exists\n", fin_dir);
        write(1, &outp, strlen(outp));
    } else {
        sprintf(outp, "[STARTING]  Error creating folder %s for finished executions", fin_dir);
        perror(outp);
    }

    // Ciclo infinito para ler pedidos dos clientes
    while(1){

        // Abrir o pipe criado
        fd_cliSer = open(fifo_cliSer_name, O_RDONLY);
        if(fd_cliSer == -1) {
            perror("[STARTING]  Error opening Client->Server pipe to read");
            exit(-1);
        }
        
        // Pipe opened
        // Ler pedidos dos clientes de tamanho fixo sizeof(struct exec)
        struct exec query;
        while((res_readfifo = read(fd_cliSer, &query, sizeof(struct exec))) > 0 && res_readfifo <= P_BUF){

            print_exec(query);

            //! EXECUTE SINGLE or EXECUTE PIPELINE
            if(query.query_int == 1 || query.query_int == 2 || query.query_int == 3) { 
                int store_request_id = request_id++;
                if(query.query_int == 1 || query.query_int == 2) {
                    sprintf(outp, "[REQUEST #%d]  New execute request\n", store_request_id);
                    write(1, &outp, strlen(outp));

                    // Guardar informação do pedido necessário
                    int pid = query.pid;
                    char prog_name[100];
                    strcpy(prog_name, query.prog_name);
                    struct timeval start = query.start;                    
                    
                    long start_ms = start.tv_sec * 1000 + start.tv_usec / 1000;
                    sprintf(outp, "[REQUEST #%d]  EXECUTE START: PID %d ║ Command \"%s\" ║ Start timeval %ld\n", store_request_id, pid, prog_name, start_ms);
                    write(1, &outp, strlen(outp));

                    // Adicionar programa à lista de execuções atuais
                    add_exec(pid, prog_name, start);
                }

                //! EXECUTE END
                else if (query.query_int == 3) {
                    // Guardar informação do pedido necessário
                    int pid = query.pid;
                    char prog_name[100];
                    strcpy(prog_name, query.prog_name);
                    struct timeval end = query.end;
                    struct timeval start = query.start;

                    // Verificar se há programas a executar que possam ser terminados (algo desnecessário)
                    int sizell = size_llexec();
                    write(fd_serCli, &sizell, sizeof(int));
                    if(sizell == 0) {
                        sprintf(outp, "[REQUEST #%d]  EXECUTE END: There aren't any programs currently running\n", store_request_id);
                        write(1, &outp, strlen(outp));
                        exit(0);
                    }

                    // Calcular tempo de execução em ms
                    long end_ms = end.tv_sec * 1000 + end.tv_usec / 1000;
                    long elapsed_time = calculate_elapsed_time(start, end);
                    sprintf(outp, "[REQUEST #%d]  EXECUTE END: PID %d ║ End timeval %ld ║ Total time %ld ms\n", store_request_id, pid, end_ms, elapsed_time);
                    write(1, &outp, strlen(outp));

                    // Remover execução da lista ligada de execuções ativas
                    remove_exec(pid, end);
                    
                    sprintf(outp, "[REQUEST #%d]  Ended execute request\n", store_request_id);
                    write(1, &outp, strlen(outp));
                }
            }

            else {
                int store_request_id = request_id++;

                // Abrir o fifo de comunicação Servidor->Cliente identificado pelo pid do pai
                int pid_pai = query.pid_pai;
                char fifo_serCli_name[strlen(fifo_serCli)+sizeof(int)];
                sprintf(fifo_serCli_name, "../tmp/%s_%d", fifo_serCli, pid_pai);               

                fd_serCli = open(fifo_serCli_name, O_WRONLY);
                if(fd_serCli == -1){
                    sprintf(outp, "[REQUEST #%d]  Error opening Server->Client pipe to write", store_request_id);
                    perror(outp);
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
                        sprintf(outp, "[REQUEST #%d]  New status request\n", store_request_id);
                        write(1, &outp, strlen(outp));

                        int resf = fork();
                        if (resf == -1) {
                            perror("Error creating child process to handle Status request");
                            exit(-1);
                        }
                        else if (resf == 0) {

                            // Verificar se há algum programa em execução
                            int sizell = size_llexec();
                            write(fd_serCli, &sizell, sizeof(int));
                            if (sizell == 0) {
                                sprintf(outp, "[REQUEST #%d]  There aren't any programs currently running\n", store_request_id);
                                write(1, &outp, strlen(outp));
                                exit(0); // TODO fecha o processo child?
                            }

                            // Escrever para o fifo de comunicação Servidor->Cliente as execuções atuais
                            for (llexec *tmp = currExecs; tmp != NULL; tmp = tmp->next) {
                                struct timeval til_now;
                                gettimeofday(&til_now, NULL);
                                struct timeval start = tmp->exec_info.start;

                                long elapsed_time = calculate_elapsed_time(start, til_now);
                                sprintf(outp, "%d   %s   %ld ms\n", tmp->exec_info.pid, tmp->exec_info.prog_name, elapsed_time);
                                int exec_size = strlen(outp);
                                write(fd_serCli, &exec_size, sizeof(int));
                                write(fd_serCli, &outp, exec_size);
                                write(1, &outp, exec_size);
                            }

                            _exit(0);
                        }
                        else {
                            wait(NULL);
                        }

                        sprintf(outp, "[REQUEST #%d]  Ended status request\n", store_request_id);
                        write(1, &outp, strlen(outp));
                    }


                    //! STATS-TIME
                    else if (query.query_int == 5) {
                        sprintf(outp, "[REQUEST #%d]  New stats-time request\n", store_request_id);
                        write(1, &outp, strlen(outp));

                        // Guardar informação do pedido necessário
                        int pids_search_size = query.pids_search_size;
                        int pids_search[pids_search_size];
                        for (int i = 0; i < pids_search_size; i++) {
                            pids_search[i] = query.pids_search[i];
                        }

                        // Calcular tempo total
                        int total_time = search_time_finished(pids_search, pids_search_size, store_request_id);

                        // Informar o Cliente do tempo total de execução dos programas
                        sprintf(outp, "Total execution time is %d ms\n", total_time);
                        int tot_size = strlen(outp);
                        write(fd_serCli, &tot_size, sizeof(int));
                        write(fd_serCli, &outp, tot_size);
                        sprintf(outp, "[REQUEST #%d]  Total execution time is %d ms\n", request_id, total_time);
                        write(1, &outp, strlen(outp));

                        sprintf(outp, "[REQUEST #%d]  Ended stats-time request\n", store_request_id);
                        write(1, &outp, strlen(outp));
                    }


                    //! STATS-COMMAND
                    else if (query.query_int == 6) {
                        sprintf(outp, "[REQUEST #%d]  New stats-command request\n", store_request_id);
                        write(1, &outp, strlen(outp));

                        // Guardar informação do pedido necessário
                        char prog_name[100];
                        strcpy(prog_name, query.prog_name);

                        int pids_search_size = query.pids_search_size;
                        int pids_search[pids_search_size];
                        for (int i = 0; i < pids_search_size; i++) {
                            pids_search[i] = query.pids_search[i];
                        }

                        // Calcular numero de vezes total
                        int total_execs_prog = search_command_finished(pids_search, pids_search_size, prog_name, store_request_id);

                        // Informar o Cliente do número de vezes que o programa foi executado
                        sprintf(outp, "%s was executed %d times\n", prog_name, total_execs_prog);
                        int tot_size = strlen(outp);
                        write(fd_serCli, &tot_size, sizeof(int));
                        write(fd_serCli, &outp, tot_size);

                        sprintf(outp, "[REQUEST #%d]  Ended stats-command request\n", store_request_id);
                        write(1, &outp, strlen(outp));
                    }


                    //! STATS-UNIQ
                    else if (query.query_int == 7) {
                        sprintf(outp, "[REQUEST #%d]  New stats-uniq request\n", store_request_id);
                        write(1, &outp, strlen(outp));

                        // Guardar informação do pedido necessário
                        int pids_search_size = query.pids_search_size;
                        int pids_search[pids_search_size];
                        for (int i = 0; i < pids_search_size; i++) {
                            pids_search[i] = query.pids_search[i];
                        }
    
                        // Informar o Cliente dos programas únicos executados pelos pids fornecidos
                        search_uniq_finished(pids_search, pids_search_size, fd_serCli, store_request_id);

                        sprintf(outp, "[REQUEST #%d]  Ended stats-uniq request\n", store_request_id);
                        write(1, &outp, strlen(outp));
                    }
                    _exit(0);
                }
                unlink(fifo_serCli_name);
                close(fd_serCli);
            }
        }
    }
    
    close(fd_cliSer);
    unlink(fifo_cliSer_name);
    return 0;
}