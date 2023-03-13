#include "guiao2.h"

//ex1
void getpids(){
    printf("PID processo filho: %d\n", getpid());
    printf("PID processo pai: %d\n", getppid());
}

//ex2
void ex2(){
    pid_t pid;
    int status;
    if((pid = fork()) == 0){
        //codigo do processo filho
        printf("[FILHO] PID: %d\n", getpid());
        printf("[FILHO] PID Pai: %d\n", getppid());
    } else {
        //codigo do processo pai
        printf("[PAI] PID : %d\n", getpid());
        printf("[PAI] PID Pai: %d\n", getppid());
        printf("[PAI] PID Filho: %d\n", pid);
    }
}



//ex3
void ex3(){
    for(int i = 0; i<19; i++){
        if((fork())==0){
            printf("...");
            sleep(10);
            _exit(0);
        } else {
            printf("nao passei");
        }
    }
}

//ex4
int ex4() {
    pid_t pid;
    int nproc = 5;
    int status;

    for(int i = 1; i < nproc; i++){
        if((pid = fork())==0){
            printf("[proc #%d] pid: %d\n", i, getpid());
            sleep(10);
            _exit(0);
        } else {
            //for(i = 0)
            printf("nao passei");
        }
    }
}
//main
int main(){
    //ex1
    getpids();
    //ex2
    ex2();

    //ex4
    //ex4();

    //ex5
    return 0;
}