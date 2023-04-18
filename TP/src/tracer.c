#include <stdio.h>      //sprintf
#include <sys/types.h>  //fifos, open
#include <sys/stat.h>   //fifos, open
#include <sys/wait.h>   //wait
#include <stdlib.h>     //exit
#include <unistd.h>     //close
#include <string.h>     //strlen, strcmp
#include <fcntl.h>      //open
#include <errno.h>      //errno
#include <time.h>       //clocks_per_sec

#define fifo_cliSer "fifo_client_server"
#define fifo_serCli "fifo_server_client"

int fd_clientServer;
int fd_serverClient;
char fifoname_write[50];
char fifoname_read[50];



//$ ./tracer execute -u "prog-a arg-1 (...) arg-n"
//Running PID 3090
//(output do programa)
//Ended in 400 ms
int execute_single(char* command){
    int i = 0;
    int resf;
    char* exec_args[32];
    int res_exec;
    char fifo_outp[100], outp[100];
    clock_t start;
    clock_t end;
    double cpu_time_used;
    int res, status;

    //adicionar esta struct ao header file conjunto?
    //verificar que é menor que o buff_size
    //timeval better than clock_t? microsecond precision but real-world time use and not CPU
/*     typedef struct Info_server {
        int pid_exec;
        char prog_name[100];
        clock_t start;
    } Info_server; */ //or struct timeval start, end;?
    

    //parse dos argumentos
    char* string = strtok(command, " ");
    while (string != NULL) {
        exec_args[i] = string;
        //printf("%s ", exec_args[i]); //DEBUG a adicionar ao servidor
        string = strtok(NULL, " ");
        i++;
    }
    exec_args[i] = NULL;

    resf = fork();
    if(resf == 0){
        //fazer esperar pela confirmacao do pai para avancar

        res_exec = execvp(exec_args[0], exec_args);
        _exit(res_exec);
    } else {
        //fazer prep e enviar info ao pipe
        int query_int = 10;
        write(fd_clientServer, &query_int, sizeof(int));
        //INFORMAR O SERVIDOR
        // -pid, nome do prog (command), timestamp inicial
        write(fd_clientServer, &resf, sizeof(int));
        int prog_name_size = strlen(exec_args[0]);
        write(fd_clientServer, &prog_name_size, sizeof(int));
        write(fd_clientServer, exec_args[0], prog_name_size);

        start = clock();
        write(fd_clientServer, &start, sizeof(clock_t));

        //INFORMAR O UTILIZADOR
        sprintf(outp, "Running PID %d\n", resf);
        write(1, &outp, strlen(outp));




        //enviar a dizer que pode comecar atraves de um pipe anonimo??

        //fazer wait e enviar info ao server
        if(resf != -1){
            waitpid(resf, &status, 0);
            if(WIFEXITED(status)) {
                res = WEXITSTATUS(status); //adicionar feedback de erro
                end = clock();
                cpu_time_used = (((double) (end - start)) / CLOCKS_PER_SEC) * 1000;
                //INFORMAR O SERVIDOR
                // -pid, timestamp final
                write(fd_clientServer, &resf, sizeof(int));
                write(fd_clientServer, &end, sizeof(clock_t));
                //INFORMAR O UTILIZADOR
                sprintf(outp, "Ended in %f ms\n", cpu_time_used); //secs to ms
                write(1, &outp, strlen(outp));
            } else
                res = -1;
        } else res = -1;
    }
    

    
    //close(fd_clientServer);
    return 0;
}

/* $ ./tracer execute -p "prog-a arg-1 (...) arg-n | prog-b arg-1 (...) arg-n | prog-c arg-1 (...) arg-n"
Running PID 3083
(output do programa)
Ended in 730 ms */
int execute_pipeline(char* command){
    return 0;
}

/* 
$ ./tracer status
3033 prog-c 10 ms
3320 prog-h 20 ms
3590 prog-d | prog-e | prog z 100ms */
int status(){
    char outp[300];

    //fazer prep e enviar query_int para o monitor
    int query_int = 30;
    write(fd_clientServer, &query_int, sizeof(int));

    //enviar para o monitor o nome do pipe criado para este pid
    int fifoname_read_size = strlen(fifoname_read);
    write(fd_clientServer, &fifoname_read_size, sizeof(int));
    write(fd_clientServer, &fifoname_read, fifoname_read_size);

    //abrir o pipe criado para este pid para ler info enviada pelo monitor
    fd_serverClient = open(fifoname_read, O_RDONLY);
    if(fd_serverClient == -1){
        perror("Error opening Server->Client pipe to read");
        exit(-1);
    }

    int sizell;
    read(fd_serverClient, &sizell, sizeof(int));
    if(sizell == 0){
        sprintf(outp, "[STATUS] That aren't any programs currently running\n");
        write(1, &outp, strlen(outp));
        exit(0);
    } else {
        for(int i = 0; i < sizell; i++){
            int exec_size;
            read(fd_serverClient, &exec_size, sizeof(int));
            char exec[exec_size];
            read(fd_serverClient, &exec, exec_size);
            write(1, &exec, exec_size);
        }
    }

    //receber struct 
    return 0;
}

/* $ ./tracer stats-time PID-1 PID-2 (...) PID-N
Total execution time is 1000 ms */
int stats_time(){
    return 0;
}

/* $ ./tracer stats-command prog-a PID-1 PID-2 (...) PID-N
Prog-a was executed 4 times */
int stats_command(){
    return 0;
}

/* $ ./tracer stats-uniq PID-1 PID-2 (...) PID-N
prog-a
prog-c
prog-h */
int stats_uniq(){
    return 0;
}

int createFIFO(){
    //fifoname_read[50];

    sprintf(fifoname_read, "../tmp/%s_%d", fifo_serCli, getpid());
    int res_createfifo = mkfifo(fifoname_read, 0660);
    if(res_createfifo == -1){
        if(errno != EEXIST) {
            perror("Error creating Server->Client pipe");
            return -1;
        }
    }
    return 0;
}


int main(int argc, char* argv[]){
    //fifoname_write[50];
    char outp[300];

    //abrir fifo para enviar msgs ao server
    sprintf(fifoname_write, "../tmp/%s", fifo_cliSer);
    fd_clientServer = open(fifoname_write, O_WRONLY);
    if(fd_clientServer == -1){
        perror("Error opening Client->Server pipe to write");
        exit(-1); //flushing output buffers and closing open files, better than return
    }

    //criar fifo servidor -> cliente
    if(createFIFO() != 0) exit(-1);

    //parse dos args da bash

    if (argc == 1){
        sprintf(outp, "Invalid tracer request. Please try again...\n");
        write(1, outp, sizeof(char) * strlen(outp));
        exit(-1);
    }

    char* query = argv[1];
    if(strcmp(query, "execute") == 0 && argc >= 2) {
        if(argc == 3 || argc == 2) {
            sprintf(outp, 
"Invalid tracer request. Please try again...\n\
Usage:\n\
        ./tracer execute -u \"prog arg1 (...) argN\"\n\
or      ./tracer execute -p \"progA arg1 (...) argN | progB arg1 (...) argN\"\n");
            write(1, outp, sizeof(char) * strlen(outp));
            exit(-1);
        }
        char* flag = argv[2];
        char* command = argv[3];
        if(strcmp(flag, "-u") == 0) execute_single(command);
        if(strcmp(flag, "-p") == 0) execute_pipeline(command);
    }
    else if(strcmp(query, "status") == 0 && argc == 2) 
            status();
    else if(strcmp(query, "stats-time") == 0 && argc >= 4) {
        //Array de PIDs argumento
        int pids[argc-2];
        for(int i = 0; i < argc; i++) 
            pids[i] = atoi(argv[i+2]);
        stats_time(argc-2, pids);
    }
    else if(strcmp(query, "stats-command") == 0 && argc >= 3) {
        char* command = argv[3];

        stats_command(command);
    }
    else if(strcmp(query, "stats-uniq") == 0 && argc >= 3) {
        //Array de PIDs argumento
        int pids[argc-2];
        for(int i = 0; i < argc; i++)
            pids[i] = atoi(argv[i+2]);
        stats_uniq(argc-2, pids);
    }

    return 0;
}