#include <stdio.h> //remover soon enough
#include <sys/types.h>  //fifos, open
#include <sys/stat.h>   //fifos, open
#include <sys/wait.h>   //wait
#include <stdlib.h>     //exit
#include <string.h>     //strcmp
#include <unistd.h>     //close
#include <fcntl.h>      //open
#include <errno.h>      //errno

#define fifo_cliSer "fifo_client_server"
#define fifo_serCli "fifo_server_client"

int fd_clientServer;

//$ ./tracer execute -u "prog-a arg-1 (...) arg-n"
//Running PID 3090
//(output do programa)
//Ended in 400 ms
int execute_single(char* command){
    int i = 0;
    int resf;
    char* exec_args[32];
    int res_exec;
    int res;
    int status;
    char fifo_outp[100], outp[100];
    double cpu_time_used

    //verificar que é menor que o buff_size
    //timeval better than clock_t? microsecond precision but real-world time use and not CPU
    typedef struct Info_server {
        char fifo_outp2[100];
        int pid_exec;
        clock_t start;
    } Info_server; //or struct timeval start, end;?


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
        res_exec = execvp(exec_args[0], exec_args);
        _exit(res_exec);
    }

    //INFORMAR O SERVIDOR
    // -pid, nome do prog (command), timestamp
    sprintf(fifo_outp, "exec_single");
    write(fd_clientServer, fifo_outp, sizeof(char)*strlen(fifo_outp)); //envia 100 ou apenas o exec_single?
    write(fd_clientServer, &resf, sizeof(resf));
    start = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    write(fd_clientServer, exec_args[0], strlen(exec_args[0])*sizeof(char));

    //INFORMAR O UTILIZADOR
    sprintf(outp, "Running PID %d\n", resf);
    write(1, outp, strlen(outp)*sizeof(char));
    return res;

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
    char fifoname[50];

    sprintf(fifoname, "/tmp/%s_%d", fifo_serCli, getpid());
    int res_createfifo = mkfifo(fifoname, 0660);
    if(res_createfifo == -1){
        if(errno != EEXIST) {
            perror("Error creating Server->Client pipe");
            return -1;
        }
    }
    return 0;
}


int main(int argc, char* argv[]){
    char fifoname[50];
    char outp[150];

    //abrir fifo para enviar msgs ao server
    sprintf(fifoname, "/tmp/%s", fifo_cliSer);
    int fd_clientServer = open(fifoname, O_WRONLY);
    if(fd_clientServer == -1){
        perror("Error opening Client->Server pipe");
        exit(-1);
    }

    //criar fifo servidor -> cliente
    if(createFIFO() != 0) exit(-1);

    //parse dos args da bash
    char* query = argv[1];
    

    if(strcmp(query, "execute") == 0 && argc == 4) {
        char* flag = argv[2];
        char* command = argv[3];

        if(strcmp(flag, "-u")) execute_single(command);
        if(strcmp(flag, "-p")) execute_pipeline(command);
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
    else {
        sprintf(outp, "Invalid tracer request. Please try again...\n");
        write(1, outp, sizeof(char)*strlen(outp));
        exit(-1);
    }

    close(fd_clientServer);
    return 0;
}