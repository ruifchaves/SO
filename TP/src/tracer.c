#include "common.h"

int fd_cliSer;
int fd_serCli;
int pid_pai;
char fifo_cliSer_name[50];
char fifo_serCli_name[50];


//! EXECUTE END
int execute_end(struct exec query, struct timeval start, struct timeval end) {
    char outp[100];

    // Informar o Servidor: Query atualizada (tipo 3 com timestamp final)
    query.query_int = 3;
    query.end = end;
    write(fd_cliSer, &query, sizeof(struct exec));

    // Informar o Ciente: Tempo de Execução
    long elapsed_time = calculate_elapsed_time(start, end);
    sprintf(outp, "Ended in %ld ms\n", elapsed_time);
    write(1, &outp, strlen(outp));
    close(fd_cliSer);
    return 0;
}

//! EXECUTE SINGLE
/* $ ./tracer execute -u "prog-a arg-1 (...) arg-n"
 Running PID 3090
(output do programa)
 Ended in 400 ms */
int execute_single(char *command) {
    int i = 0;
    int resf, res_exec;
    char *prog_args[32];
    char outp[100];
    struct timeval start, end;
    int wait_result, res, status;

    // Parse do programa e respetivos argumentos
    char *command_copy = strdup(command);
    char *string = strtok(command, " ");
    while (string != NULL) {
        prog_args[i] = string;
        string = strtok(NULL, " ");
        i++;
    }
    prog_args[i] = NULL;

    // Guardar timestamp inicial
    gettimeofday(&start, NULL);

    // Fazer fork para executar o comando
    resf = fork();
    if (resf == -1) {
        perror("Error in fork");
        return -1;
    }
    else if (resf == 0) {
        res_exec = execvp(prog_args[0], prog_args);
        _exit(res_exec);
    }
    else {
        // Informar o Servidor: Nova Query (tipo 1 - pid, nome do programa e timestamp inicial)
        struct exec tmp = {1, pid_pai, resf, "", start, {0}, {0}, 0};
        strcpy(tmp.prog_name, command_copy);
        write(fd_cliSer, &tmp, sizeof(struct exec));

        // Informar o cliente: PID
        sprintf(outp, "Running PID %d\n", resf);
        write(1, &outp, strlen(outp));

        // Esperar que a execução do processo filho termine
        wait_result = waitpid(resf, &status, 0);
        if (wait_result != -1 && WIFEXITED(status)) {
            res = WEXITSTATUS(status); //TODO adicionar feedback de erro
            // Guardar timestamp final
            gettimeofday(&end, NULL);

            // Iniciar final de execução
            execute_end(tmp, start, end);
        }
        else {
            perror("Error in waitpid or child process");
            return -1;
        }
    }

    // Fechar file descriptors e libertar memória
    free(command_copy);
    close(fd_cliSer);
    return 0;
}


//! EXECUTE PIPELINE
// strace -f -o trace.log ./tracer execute -p "ls -la | grep mon | wc"
/* $ ./tracer execute -p "prog-a arg-1 (...) arg-n | prog-b arg-1 (...) arg-n | prog-c arg-1 (...) arg-n"
Running PID 3083
(output do programa)
Ended in 730 ms */
// EXAMPLE: grep -v ^#/etc/passwd | cut -f7 -d: | uniq | wc -l
int execute_pipeline(char *command) {
    int resf, res_exec, status;
    int num_progs = 0, num_args = 0, arg = 0;
    char *prog_args[32];
    char *args[32][32];
    char outp[100];
    struct timeval start, end;

    // Parse dos programas da pipeline
    char *command_copy = strdup(command);
    char *string = strtok(command, "|");
    while (string != NULL)     {
        int str_size = strlen(string) - 1;
        // Remover os espacos de antes e depois do |
        if (isspace(string[str_size]))
            string[str_size] = '\0';
        if (isspace(string[0]))
            string = string + 1;
        prog_args[num_progs] = string;
        string = strtok(NULL, "|");
        num_progs++;
    }
    prog_args[num_progs] = NULL;

    // Parse dos argumentos dos programas
    for (arg = 0; arg < num_progs; arg++)     {
        char *string = strtok(prog_args[arg], " ");
        num_args = 0;
        while (string != NULL) {
            args[arg][num_args] = string;
            string = strtok(NULL, " ");
            num_args++;
        }
        args[arg][num_args] = NULL;
    }

    // Criar o array que vai conter os file descriptors dos pipes
    int num_pipes = num_progs - 1;
    int pipes[num_pipes][2];

    // Criar os pipes de comunicação entre processos filho
    for (int i = 0; i < num_pipes; i++) {
        if (pipe(pipes[i]) < 0) {
            perror("Error creating pipes");
            exit(1);
        }
    }
    
    // Fazer um fork para executar cada comando da pipeline
    int pids[num_progs];
    for (int forkid = 0; forkid < num_progs; forkid++) {
        if (forkid == 0) gettimeofday(&start, NULL);

        resf = fork();
        if (resf == -1) {
            sprintf(outp, "Error in fork #%d", forkid+1);
            perror(outp);
            return -1;
        }
        else if (resf == 0) {

            if (forkid == 0) {                              //TODO make following comments readable
                close(pipes[forkid][0]);                    // Fechar pipes[0][0]; Vai ler do stdin
                dup2(pipes[forkid][1], STDOUT_FILENO);      // Mudar stdout para pipes[0][1]
            }
            else if (forkid == num_pipes) {
                close(pipes[forkid - 1][1]);                // Fechar pipes[last_pipe][1]; Vai escrever no stdout
                dup2(pipes[forkid - 1][0], STDIN_FILENO);   // Mudar stdin para pipes[last_pipe][0]
            }
            else {
                close(pipes[forkid - 1][1]);                // Fechar pipes[prev_pipe][1]; Fecha-se o escritor
                dup2(pipes[forkid - 1][0], STDIN_FILENO);   // Mudar stdin para pipes[prev_pipe][0]
                close(pipes[forkid][0]);                    // Fechar pipes[curr_pipe][1]; Fecha-se o escritor
                dup2(pipes[forkid][1], STDOUT_FILENO);      // Mudar stdout para pipes[curr_pipe][1]
            }

            // Fechar todos os pipes porque ou não são necessários ou já foram duplicados
            for (int cl = 0; cl < num_pipes; cl++) {
                close(pipes[cl][0]);
                close(pipes[cl][1]);
            }

            // Executar o comando respetivo da pipeline
            res_exec = execvp(args[forkid][0], args[forkid]);
            _exit(res_exec);
        }
        else pids[forkid] = resf;
    }

    // Fechar todos os pipes que poderão ter ficado abertos
    for (int i = 0; i < num_pipes; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    
    struct exec tmp;
    // Esperar que todos os processos filho terminem
    int exitid;
    for (exitid = 0; exitid < num_progs; exitid++) {
        int status, fst_pid;
        if (exitid == 0) {

            // Informar o Servidor: Nova Query (tipo 2 - pid 1º processo, nome da pipeline e timestamp inicial)
            tmp = (exec) {2, pid_pai, pids[0], "", start, {0}, {0}, 0};
            strcpy(tmp.prog_name, command_copy);
            write(fd_cliSer, &tmp, sizeof(struct exec));

            // Informar o cliente: PID
            sprintf(outp, "Running PID %d\n", pids[0]);
            write(1, &outp, strlen(outp));
        }
        waitpid(pids[exitid], &status, 0);
    }

    // Caso corresponda ao último processo da pipeline
    if (exitid == num_progs) {
        // Guardar timestamp final
        gettimeofday(&end, NULL);
        // Iniciar final de execução
        execute_end(tmp, start, end);
    }

    // Fechar file descriptors e libertar memória
    free(command_copy);
    return 0;
}


//! STATUS
/* $ ./tracer status
3033 prog-c 10 ms
3320 prog-h 20 ms
3590 prog-d | prog-e | prog z 100ms */
int status() {
    char outp[300];

    // Informar o Servidor: Nova Query (tipo 4 - pid)
    struct exec tmp = {4, pid_pai, pid_pai, "", {0}, {0}, {0}, 0};
    write(fd_cliSer, &tmp, sizeof(struct exec));

    // Abrir o pipe criado para ler msgs do servidor
    fd_serCli = open(fifo_serCli_name, O_RDONLY);
    if (fd_serCli == -1) {
        perror("Error opening Server->Client pipe to read");
        exit(-1);
    }

    // Receber num de progs a correr e, caso haja, os seus nomes
    int sizell;
    read(fd_serCli, &sizell, sizeof(int));
    if (sizell == 0) {
        sprintf(outp, "There aren't any programs currently running\n");
        write(1, &outp, strlen(outp));
        exit(0);
    }
    else {
        // Receber informação relativa a cada programa em execução
        for (int i = 0; i < sizell; i++) {
            int exec_size;
            read(fd_serCli, &exec_size, sizeof(int));
            char exec[exec_size];
            read(fd_serCli, &exec, exec_size);
            write(1, &exec, exec_size);
        }
    }

    return 0;
}


//! STATS-TIME
/* $ ./tracer stats-time PID-1 PID-2 (...) PID-N
Total execution time is 1000 ms */
int stats_time(int *pids, int pids_size) {
    char outp[300];

    // Informar o Servidor: Nova Query (tipo 5 - pid, pids a pesquisar e quantidade dos mesmos)
    struct exec tmp = {5, pid_pai, pid_pai, "", {0}, {0}, {0}, pids_size};
    for (int i = 0; i < pids_size; i++) {
        tmp.pids_search[i] = pids[i];
    }
    write(fd_cliSer, &tmp, sizeof(struct exec));


    // Abrir o pipe criado para ler msgs do servidor
    fd_serCli = open(fifo_serCli_name, O_RDONLY);
    if (fd_serCli == -1) {
        perror("Error opening Server->Client pipe to read");
        exit(-1);
    }


    // Receber a string com o tempo total de execução dos mesmos em ms
    int tot_size;
    read(fd_serCli, &tot_size, sizeof(int));
    char str[tot_size];
    read(fd_serCli, &str, tot_size);
    write(1, &str, tot_size);

    return 0;
}


//! STATS-COMMAND
/* $ ./tracer stats-command prog-a PID-1 PID-2 (...) PID-N
Prog-a was executed 4 times */
int stats_command(char *command, int *pids, int pids_size) {
    char outp[300];

    // Informar o Servidor: Nova Query (tipo 6 - pid, nome do programa, pids a pesquisar e quantidade dos mesmos)
    struct exec tmp = {6, pid_pai, pid_pai, "", {0}, {0}, {0}, pids_size};
    for (int i = 0; i < pids_size; i++) {
        tmp.pids_search[i] = pids[i];
    }
    strcpy(tmp.prog_name, command);
    write(fd_cliSer, &tmp, sizeof(struct exec));


    // Abrir o pipe criado para ler msgs do servidor
    fd_serCli = open(fifo_serCli_name, O_RDONLY);
    if (fd_serCli == -1) {
        perror("Error opening Server->Client pipe to read");
        exit(-1);
    }

    // Receber a string com o número de vezes que o commando executou
    int tot_size;
    read(fd_serCli, &tot_size, sizeof(int));
    char str[tot_size];
    read(fd_serCli, &str, tot_size);
    write(1, &str, tot_size);

    return 0;
}


//! STATS-UNIQ
/* $ ./tracer stats-uniq PID-1 PID-2 (...) PID-N
prog-a
prog-c
prog-h */
int stats_uniq(int *pids, int pids_size) {
    char outp[300];

    // Informar o Servidor: Nova Query (tipo 7 - pid, pids a pesquisar e quantidade dos mesmos)
    struct exec tmp = {7, pid_pai, pid_pai, "", {0}, {0}, {0}, pids_size};
    for (int i = 0; i < pids_size; i++) {
        tmp.pids_search[i] = pids[i];
    }
    write(fd_cliSer, &tmp, sizeof(struct exec));

    // Abrir o pipe criado para ler msgs do servidor
    fd_serCli = open(fifo_serCli_name, O_RDONLY);
    if (fd_serCli == -1) {
        perror("Error opening Server->Client pipe to read");
        exit(-1);
    }

    // Receber o número de programas únicos e, caso haja, os seus nomes
    int tot_size;
    read(fd_serCli, &tot_size, sizeof(int));
    for(int i = 0; i < tot_size; i++){
        int tmp_size;
        read(fd_serCli, &tmp_size, sizeof(int));
        char tmp[tmp_size];
        read(fd_serCli, &tmp, tmp_size);
        write(1, &tmp, tmp_size);
    }

    return 0;
}






// Criar fifo Servidor->Cliente com pid como identificador
int createFIFO() {
    sprintf(fifo_serCli_name, "../tmp/%s_%d", fifo_serCli, getpid());
    int res_createfifo = mkfifo(fifo_serCli_name, 0660);
    if (res_createfifo == -1) {
        if (errno != EEXIST) {
            perror("Error creating Server->Client pipe");
            return -1;
        }
    }
    return 0;
}

int main(int argc, char *argv[]) {
    char outp[300];
    int q_ret;

    if(argc == 1){
        sprintf(outp, "Invalid tracer request.\nPlease try again...\n");
        write(1, outp, strlen(outp));
        exit(-1);
    }

    pid_pai = getpid();

    // Abrir fifo Cliente -> Servidor
    sprintf(fifo_cliSer_name, "../tmp/%s", fifo_cliSer);
    fd_cliSer = open(fifo_cliSer_name, O_WRONLY);
    if (fd_cliSer == -1) {
        perror("Error opening Client->Server pipe to write");
        return -1;
    }

    // Criar fifo Servidor->Cliente
    if (createFIFO() != 0)
        return -1;
    
    // Redirecionamento consoante argumentos
    char *query = argv[1];
    if (strcmp(query, "execute") == 0 && argc >= 4) {
        char *flag = argv[2];
        char *command = argv[3];
        //! EXECUTE SINGLE
        if (strcmp(flag, "-u") == 0)
            execute_single(command);

        //! EXECUTE PIPELINE
        char* cmd = strdup(command);
        if (strcmp(flag, "-p") == 0 && strtok(cmd, "|") != NULL){
            execute_pipeline(command);
            free(cmd);
        }
    }
    
    //! STATUS
    else if (strcmp(query, "status") == 0 && argc == 2)
        status();
    
    //! STATS-TIME
    // Assume-se que basta pelo menos um pid como argumento
    else if (strcmp(query, "stats-time") == 0 && argc >= 3) {
        // Criar array de PIDs argumento
        int pids[argc - 2];
        for (int i = 0; i < argc - 2; i++)
            pids[i] = atoi(argv[i + 2]);
        q_ret = stats_time(pids, argc - 2);
        if(q_ret == -1){
            sprintf(outp, "Error in Stats-time\n");
            write(1, outp, strlen(outp));
        }
    }

    //! STATS-COMMAND
    // Assume-se que basta pelo menos um pid como argumento
    else if (strcmp(query, "stats-command") == 0 && argc >= 4) {
        char *command = argv[2];
        
        // Criar array de PIDs argumento
        int pids[argc - 3];
        for (int i = 0; i < argc - 3; i++)
            pids[i] = atoi(argv[i + 3]);
        q_ret = stats_command(command, pids, argc - 3);
        if(q_ret == -1){
            sprintf(outp, "Error in Stats-command\n");
            write(1, outp, strlen(outp));
        }
    }
        
    //! STATS-UNIQ
    // Assume-se que basta pelo menos um pid como argumento
    else if (strcmp(query, "stats-uniq") == 0 && argc >= 3) {
        // Criar array de PIDs argumento
        int pids[argc - 2];
        for (int i = 0; i < argc - 2; i++)
            pids[i] = atoi(argv[i + 2]);
        q_ret = stats_uniq(pids, argc - 2);
        if(q_ret == -1){
            sprintf(outp, "Error in Stats-uniq\n");
            write(1, outp, strlen(outp));
        }
    } 
    else {
        sprintf(outp, "Invalid tracer request.\nPlease try again...\n");
        write(1, outp, strlen(outp));
        exit(-1);
    }

    unlink(fifo_serCli_name);
    return 0;
}