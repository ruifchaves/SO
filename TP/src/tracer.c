#include "common.h"

int fd_clientServer;
int fd_serverClient;
int pid_pai;
char fifoname_write[50];
char fifoname_read[50];


//! EXECUTE END
int execute_end(struct exec query, struct timeval start, struct timeval end) {
    char outp[100];

    //printf("query: %d\n", query.pid_pai);

    // Enviar prog id para o servidor
    query.query_int = 3;
    query.end = end;
    //printf("query: %d\n", query.pid_pai);

    write(fd_clientServer, &query, sizeof(struct exec));

    long elapsed_time = calculate_elapsed_time(start, end);

    // Informar o Ciente: Tempo de Execução
    sprintf(outp, "Ended in %ld ms\n", elapsed_time); // secs to ms
    write(1, &outp, strlen(outp));
    close(fd_clientServer);
    return 0;
}

//! EXECUTE SINGLE
/* $ ./tracer execute -u "prog-a arg-1 (...) arg-n"
 Running PID 3090
(output do programa)
 Ended in 400 ms */
int execute_single(char *command) {
    int i = 0;
    int resf;
    char *exec_args[32];
    int res_exec;
    char fifo_outp[100], outp[100];
    struct timeval start;
    struct timeval end;
    double cpu_time_used;
    int wait_result, res, status;

    // Parse do programa e respetivos argumentos
    char *command_copy = strdup(command);
    char *string = strtok(command, " ");
    while (string != NULL) {
        exec_args[i] = string;
        //printf("%s ", exec_args[i]); //TODO DEBUG para o servidor?
        string = strtok(NULL, " ");
        i++;
    }
    exec_args[i] = NULL;

    gettimeofday(&start, NULL);
    // Fazer fork para executar o comando
    resf = fork();
    if (resf == -1) {
        perror("Error in fork");
        return -1;
    }
    else if (resf == 0) {
        res_exec = execvp(exec_args[0], exec_args);
        _exit(res_exec);
    }
    else {

        struct exec tmp = {1, pid_pai, resf, "", start, {0}, {0}, 0};
        strcpy(tmp.prog_name, command_copy);
        write(fd_clientServer, &tmp, sizeof(struct exec));
        // Enviar prog id para o servidor
        // Informar o Servidor: PID. nome do programa e timestamp inicial


        // Informar o cliente: PID
        sprintf(outp, "Running PID %d\n", resf);
        write(1, &outp, strlen(outp));

        // Esperar que a execução do processo filho termine
        wait_result = waitpid(resf, &status, 0);
        if (wait_result != -1 && WIFEXITED(status)) {
            res = WEXITSTATUS(status); //TODO adicionar feedback de erro
            gettimeofday(&end, NULL);

            execute_end(tmp, start, end);
        }
        else {
            perror("Error in waitpid or child process");
            return -1;
        }
    }

    // Fechar file descriptors e libertar memória
    free(command_copy);
    close(fd_clientServer);
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

    char fifo_outp[100], outp[100];
    struct timeval start, end;
    double cpu_time_used;

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
        //printf("%s ", prog_args[num_progs]); //TODO DEBUG para o servidor?
        string = strtok(NULL, "|");
        num_progs++;
    }
    prog_args[num_progs] = NULL;

    // Parse dos argumentos dos programas
    for (arg = 0; arg < num_progs; arg++)     {
        //printf("prog %d: %s\n", arg, prog_args[arg]);  //TODO DEBUG para o servidor?
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


            // Informar o Servidor: Primeiro PID e timestamp inicial
            tmp = (exec) {2, pid_pai, pids[0], "", start, {0}, {0}, 0};
            strcpy(tmp.prog_name, command_copy);
            write(fd_clientServer, &tmp, sizeof(struct exec));


            // Informar o cliente: PID
            sprintf(outp, "Running PID %d\n", pids[0]);
            write(1, &outp, strlen(outp));
        }
        waitpid(pids[exitid], &status, 0);
    }

    if (exitid == num_progs) {
        gettimeofday(&end, NULL);
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

    // Enviar prog id para o servidor

    struct exec tmp = {4, pid_pai, pid_pai, "", {0}, {0}, {0}, 0};
    write(fd_clientServer, &tmp, sizeof(struct exec));

    // Abrir o pipe criado para ler msgs do servidor
    fd_serverClient = open(fifoname_read, O_RDONLY);
    if (fd_serverClient == -1) {
        perror("Error opening Server->Client pipe to read");
        exit(-1);
    }

    // Receber num de progs a correr e, caso haja, os seus nomes
    int sizell;
    read(fd_serverClient, &sizell, sizeof(int));
    if (sizell == 0) {
        sprintf(outp, "That aren't any programs currently running\n");
        write(1, &outp, strlen(outp));
        exit(0);
    }
    else {
        for (int i = 0; i < sizell; i++) {
            int exec_size;
            read(fd_serverClient, &exec_size, sizeof(int));
            char exec[exec_size];
            read(fd_serverClient, &exec, exec_size);
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


    struct exec tmp = {5, pid_pai, pid_pai, "", {0}, {0}, {0}, pids_size};
    for (int i = 0; i < pids_size; i++) {
        tmp.pids_search[i] = pids[i];
    }
    write(fd_clientServer, &tmp, sizeof(struct exec));


    // Abrir o pipe criado para ler msgs do servidor
    fd_serverClient = open(fifoname_read, O_RDONLY);
    if (fd_serverClient == -1) {
        perror("Error opening Server->Client pipe to read");
        exit(-1);
    }

    // TODO
    // Receber num de progs terminados
    int sizell;
    // read(fd_serverClient, &sizell, sizeof(int));
    // if(sizell == 0){
    //     sprintf(outp, "That aren't any programs that have already finished\n");
    //     write(1, &outp, strlen(outp));
    //     exit(0);
    /*     } else if(sizell < pids_size){
            sprintf(outp, "There haven't been as many programs that finished as your arguments\n");
            write(1, &outp, strlen(outp));
            exit(0); */
    //} else {

    // Caso haja programas terminados. receber o tempo total de execução dos mesmos em ms
    int tot_size;
    read(fd_serverClient, &tot_size, sizeof(int));
    char str[tot_size];
    read(fd_serverClient, &str, tot_size);
    write(1, &str, tot_size);

    return 0;
}


//! STATS-COMMAND
/* $ ./tracer stats-command prog-a PID-1 PID-2 (...) PID-N
Prog-a was executed 4 times */
int stats_command(char *command, int *pids, int pids_size) {
    char outp[300];


    struct exec tmp = {6, pid_pai, pid_pai, "", {0}, {0}, {0}, pids_size};
    for (int i = 0; i < pids_size; i++) {
        tmp.pids_search[i] = pids[i];
    }
    strcpy(tmp.prog_name, command);

    write(fd_clientServer, &tmp, sizeof(struct exec));

    // Abrir o pipe criado para ler msgs do servidor
    fd_serverClient = open(fifoname_read, O_RDONLY);
    if (fd_serverClient == -1) {
        perror("Error opening Server->Client pipe to read");
        exit(-1);
    }

    // TODO
    // Receber num de progs terminados
    int sizell;
    // read(fd_serverClient, &sizell, sizeof(int));
    // if(sizell == 0){
    //     sprintf(outp, "That aren't any programs that have already finished\n");
    //     write(1, &outp, strlen(outp));
    //     exit(0);
    /*     } else if(sizell < pids_size){
            sprintf(outp, "There haven't been as many programs that finished as your arguments\n");
            write(1, &outp, strlen(outp));
            exit(0); */
    //} else {


    // Receber num de vezes que o commando executou
    int tot_size;
    read(fd_serverClient, &tot_size, sizeof(int));
    char str[tot_size];
    read(fd_serverClient, &str, tot_size);
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

    struct exec tmp = {7, pid_pai, pid_pai, "", {0}, {0}, {0}, pids_size};
    for (int i = 0; i < pids_size; i++) {
        tmp.pids_search[i] = pids[i];
    }
    write(fd_clientServer, &tmp, sizeof(struct exec));

    // Abrir o pipe criado para ler msgs do servidor
    fd_serverClient = open(fifoname_read, O_RDONLY);
    if (fd_serverClient == -1) {
        perror("Error opening Server->Client pipe to read");
        exit(-1);
    }

    // TODO
    // Receber num de progs terminados
    int sizell;
    // read(fd_serverClient, &sizell, sizeof(int));
    // if(sizell == 0){
    //     sprintf(outp, "That aren't any programs that have already finished\n");
    //     write(1, &outp, strlen(outp));
    //     exit(0);
    /*     } else if(sizell < pids_size){
            sprintf(outp, "There haven't been as many programs that finished as your arguments\n");
            write(1, &outp, strlen(outp));
            exit(0); */
    //} else {

    int tot_size;
    read(fd_serverClient, &tot_size, sizeof(int));
    for(int i = 0; i < tot_size; i++){
        int tmp_size;
        read(fd_serverClient, &tmp_size, sizeof(int));
        char tmp[tmp_size];
        read(fd_serverClient, &tmp, tmp_size);
        write(1, &tmp, tmp_size);
    }

    return 0;
}






// Criar fifo Servidor->Cliente com pid como identificador
int createFIFO() {
    sprintf(fifoname_read, "../tmp/%s_%d", fifo_serCli, getpid());
    int res_createfifo = mkfifo(fifoname_read, 0660);
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

    if(argc == 1){
        sprintf(outp, "Invalid tracer request.\nPlease try again...\n");
        write(1, outp, strlen(outp));
        exit(-1);
    }

    pid_pai = getpid();

    // Abrir fifo Cliente -> Servidor
    sprintf(fifoname_write, "../tmp/%s", fifo_cliSer);
    fd_clientServer = open(fifoname_write, O_WRONLY);
    if (fd_clientServer == -1) {
        perror("Error opening Client->Server pipe to write");
        exit(-1); // flushing output buffers and closing open files, better than return
    }

    // Criar fifo Servidor->Cliente
    if (createFIFO() != 0)
        exit(-1);
    
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
        stats_time(pids, argc - 2);
    }

    //! STATS-COMMAND
    // Assume-se que basta pelo menos um pid como argumento
    else if (strcmp(query, "stats-command") == 0 && argc >= 4) {
        char *command = argv[2];
        
        // Criar array de PIDs argumento
        int pids[argc - 3];
        for (int i = 0; i < argc - 3; i++)
            pids[i] = atoi(argv[i + 3]);
        stats_command(command, pids, argc - 3);
    }
        
    //! STATS-UNIQ
    // Assume-se que basta pelo menos um pid como argumento
    else if (strcmp(query, "stats-uniq") == 0 && argc >= 3) {
        // Criar array de PIDs argumento
        int pids[argc - 2];
        for (int i = 0; i < argc - 2; i++)
            pids[i] = atoi(argv[i + 2]);
        stats_uniq(pids, argc - 2);
    } 
    else {
        sprintf(outp, "Invalid tracer request.\nPlease try again...\n");
        write(1, outp, strlen(outp));
        exit(-1);
    }

    unlink(fifoname_read);
    return 0;
}