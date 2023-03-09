#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>  //chamadas wait e macros relaciondas
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>


int ex1(){
    int myid = getpid();
    int parentid = getppid(); //processo do terminal/linha de comandos

    printf("## My PID: %d, Parent PID: %d ##\n", myid, parentid);

    return 0;
}

int ex2(){
    //ambos estes processos resultantes do fork estao a correr em paralelo
    //como tem poucas instrucoes, acabam em ordens distintas em difs runtimes

    int res = fork();

    if(res == 0){ //[FILHO]
        int myid = getpid();
        int parentid = getppid();

        printf("[FILHO] ## My PID: %d ## Parent PID: %d ##\n", myid, parentid);
        sleep(2);
        _exit(100);

    } else { //[PAI]
        int status;
        wait(&status);

        int myid = getpid();
        int parentid = getppid();
        int filhodopai = res;

        if(WIFEXITED(status)){
            printf("[PAI] ## My PID: %d, Parent PID: %d, Son PID: %d , SON WEXITSTATUS: %d ##\n", myid, parentid, filhodopai, WEXITSTATUS(status));
        } else {
            printf("[PAI] ## Son's Process didn't finish successfully ##"); //testar isto recorrendo ao ps e kill -9 [pid]
        }
    }
    return 0;
}

int ex3(){

    for(int i=1; i<=10; i++){
        int status;

        //int res = fork();
        pid_t res = fork();
        if(res == 0){
            printf("[PROCESSO FILHO %d] ## Parent PID: %d ##\n", getpid(), getppid());
            
            sleep(1);

            _exit(i);
            //_exit() termina o processo atual com código passado por argumento.
        } else {
            //WAIT()
            //bloqueia o processo-pai até um processo-filho terminar.
            //Retorna PID do processo-filho que terminou.
            //A variável status é atualizada com o código passado na chamada da função _exit().

            /*
            The status value is a sort of composite value which includes the process "exit status" plus other bits.
            WIFEXITED: If the value of WIFEXITED(stat_val) is non-zero, this macro evaluates to the low-order 8 bits of the status argument that the child process passed to _exit() or exit(), or the value the child process returned from main(). */
            //if (wait(&status) > 0) {
            //    printf("[PROCESSO %d] ## Son's Status: %d ##\n", getpid(), status);
            //    if (WIFEXITED(status)) {
            //        printf("[PROCESSO %d] ## Son's WEXITSTATUS: %d ##\n", getpid(), WEXITSTATUS(status));
            //    } else {
            //        return status;
            //    }
            //}
            

            //WAITPID(.., .., 0 ou options)
            //<-1 meaning wait for any child process whose process group ID is equal to the absolute value of pid.
            //-1  meaning wait for any child process.
            //0   meaning wait for any child process whose process group ID is equal to that of the calling process.
            //>0  meaning wait for the child whose process ID is equal to the value of pid.
            //OPTIONS
            //WNOHANG      return immediately if no child has exited.
            //WUNTRACED    also return if a child has stopped (but not traced via ptrace(2)). Status for traced children which have stopped is provided even if this option is not specified.
            //WCONTINUED   also return if a stopped child has been resumed by delivery of SIGCONT.

            pid_t aux = waitpid(res, &status, WUNTRACED);
            printf("[PROCESSO PAI   %d] ## Son's Status: %d, waitpid: %d, WEXITSTATUS: %d\n", getpid(), status, aux, WEXITSTATUS(status));
        }
    }
    return 0;
}

int ex4(){
    for(int i=1; i<=10; i++){
        
        int res = fork();
        if(res == 0){
            printf("[PROCESSO FILHO %d] ## Parent PID: %d ##\n", getpid(), getppid());
            sleep(1);
            _exit(i);
        }
    }

    for (int i = 1; i <= 10; i++) {
        int status;
        wait(&status);
        printf("[PROCESSO PAI   %d] ## Son's Status: %d, WEXITSTATUS: %d\n", getpid(), status, WEXITSTATUS(status));
    }
    return 0;
}

int ex5(int num){
    int rows = 10;
    int columns = 1000;
    int matrix[rows][columns];

    int res;
    int randmax = 3000;

    srand(time(NULL));

    for(int i = 0; i < rows; i++){
        for (int j = 0; j < columns; j++){
            matrix[i][j] = rand() % randmax;
        }
    }
    
    printf("Matriz inicializada...\n");

    for(int lin = 0; lin < rows; lin++){
        res = fork();

        if(res == 0){
            printf("[PROCESSO FILHO %d] Searching in row %d\n", getpid(), lin+1);
            for(int col = 0; col < columns; col++){
                if(matrix[lin][col] == num){
                    _exit(lin+1);
                }
            }
            _exit(-1);
        }
    }

    for(int lin = 0; lin < rows; lin++){
        int status;
        wait(&status);
        //waitpid(pids[i], &status, 0);
        if(WEXITSTATUS(status) < 255){ //range 0 a 255, -1 excluído
            printf("[PROCESSO PAI   %d] Found in line %d!\n", getpid(), WEXITSTATUS(status));
        }
    }
    return 0;
}


int ex6(int num){ //same as above but with additions
    int rows = 10;
    int columns = 1000;
    int matrix[rows][columns];

    int res;
    int randmax = 3000;
    int pids[rows];  //new

    srand(time(NULL));

    for(int i = 0; i < rows; i++){
        for (int j = 0; j < columns; j++){
            matrix[i][j] = rand() % randmax;
        }
    }
    
    printf("Matriz inicializada...\n");

    for(int lin = 0; lin < rows; lin++){
        res = fork();

        if(res == 0){
            printf("[PROCESSO FILHO %d] Searching in row %d\n", getpid(), lin+1);
            for(int col = 0; col < columns; col++){
                if(matrix[lin][col] == num){
                    _exit(lin+1);
                }
            }
            _exit(-1);
        } else {              //new
            pids[lin] = res;  //new
        }                     //new
    }

    for(int lin = 0; lin < rows; lin++){
        int status;
        waitpid(pids[lin], &status, 0);  //new
        if(WEXITSTATUS(status) < 255){ //range 0 a 255, -1 excluído
            printf("[PROCESSO PAI   %d] Found in line %d!\n", getpid(), WEXITSTATUS(status));
        }
    }
    return 0;
}


int main(int argc, char* argv[]){
    if (argc != 2 && argc != 3) {
        printf(
"\n---------------------------------\n\
Welcome to the program!\n\
Please select an option below:\n\n\
Flag  Function\n\
----  --------\n\
-1    Print the process ID and parent process ID\n\
-2    Create a child process and print its ID and parent's ID\n\
-3    Create 10 child sequential processes and print their IDs with ordered exit statuses\n\
-4    Create 10 child parallel processes and print their IDs with ordered exit statuses\n\
-5    Create child parallel processes to search for number passed as arg in matrix (any order)\n\
-6    Create child parallel processes to search for number passed as arg in matrix (ordered)\n\
\n\
Usage: ./program [flag]\n\
Usage: ./program -5 [number]\n\
---------------------------------\n\n");
        return 1;
    }

    char* flag = argv[1];

    if(strcmp(flag,"-1") == 0) ex1();
    else if(strcmp(flag,"-2") == 0) ex2();
    else if(strcmp(flag,"-3") == 0) ex3();
    else if(strcmp(flag,"-4") == 0) ex4();
    else if(strcmp(flag,"-5") == 0) ex5(atoi(argv[2]));
    else if(strcmp(flag,"-6") == 0) ex6(atoi(argv[2]));
    else {
        printf("Invalid flag.\n");
        return 1;
    }

    return 0;
}

//gcc -Wall -g gui2.c -o gui2
//>ps -> Z+ -> processo zombie