#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>     //sprintf
#include <sys/types.h> //fifos, open
#include <sys/stat.h>  //fifos, open
#include <sys/wait.h>  //wait
#include <stdlib.h>    //exit
#include <unistd.h>    //close
#include <string.h>    //strlen, strcmp
#include <fcntl.h>     //open
#include <errno.h>     //errno
#include <sys/time.h>  //gettimeofday
#include <ctype.h>     //isspace

typedef struct exec {
    int query_int;
    int pid_pai;
    int pid;
    char prog_name[100];
    struct timeval start;
    struct timeval end;
    int pids_search[100];
    int pids_search_size;
} exec;

#define fifo_cliSer "fifo_client_server"
#define fifo_serCli "fifo_server_client"

long calculate_elapsed_time(struct timeval start, struct timeval end){
    long elapsed_seconds = end.tv_sec - start.tv_sec;
    long elapsed_useconds = end.tv_usec - start.tv_usec;
    long elapsed_time = (elapsed_seconds * 1000) + (elapsed_useconds / 1000);
    return elapsed_time;
}

#endif