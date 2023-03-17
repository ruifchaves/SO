#include <sys/wait.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

//1
//readln do stdin
//passar ao mysystem
//mysystem(string command) -> sem wait e a correr no filho, quando terminar to terminal



//2 execucao sequencial dentro de uma paralela

//C
// | -> a (se return nao for pretendido:) -> a -> a
// | -> b
// | -> c












int ex4(char* exec) {
    char* args[32]; // maximum number of arguments
    int i = 0;
    
    char* token = strtok(exec, " ");
    while (token != NULL) {
        args[i] = token;
        i++;
        token = strtok(NULL, " ");
    }
    args[i] = NULL; // set the last element to NULL
    
    execvp(args[0], args);
    
    _exit(1);
}

int main() {
    char* exec = "ls -l -a";
    ex4(exec);
    return 0;
}

int ex4(char* arg){

    char* prog = strtok(arg, " ");

    char* tmp = prog;
    for(i = 0; )


    char* flags = strtok(NULL, " ");
    int res = execvp(prog, flags);
    if(res == -1){

    }
    _exit(1);
}


int ex4_system(){

    int res = system("ls -la");

    return 0;
}


int ex3(int args, char* argv[]){

    for(int i = 0; i < args; i++){
        int res = fork();
        if(res == 0){
            sleep(1);
            int res = execlp(argv[i+2], argv[i+1], NULL);
            _exit(1);
        }
    }

    for(int i = 0; i < args; i++){
        int status;
        wait(&status);
        if(WIFEXITED(status))
            printf("%d Exited with code: %d\n", i, WEXITSTATUS(status));
        else
            printf("%d Correu mal...\n", i);
    }
    return 0;
}


int ex2(){

    int res;

    int fres = fork();
    if(fres == 0){
        //FILHO

        sleep(2);
        res = execl("/bin/ls", "ls", "-l", NULL); 
        printf("Correu mal... %d\n", res);
        _exit(1); //este exit é importante porque caso o processo corra mal, ele é terminado
    } else {
        //PAI
        wait(NULL);//forcar execucao sequencial, ao inves de paralela/concorrente
        printf("Terminei");
    }

    printf('Terminou.. %d\n', res);

    return 0;
}

//1. Implemente um programa que execute o comando ls -l. Note que no caso da execuc¸ao ser bem ˜
//sucedida, mais nenhuma outra instruc¸ao˜ e executada do programa original.
int ex1(){
    int res = execl("/bin/ls", "ls", "-l", NULL); 
    //caso o path seja correto, o codigo é substituido e o printf não é executado.
    //caso o path seja incorreto "/bin/sudhvfsv", res tem valor de retorno e o printf é executado.

    printf('Terminou.. %d\n', res);

    return 0;
}


int main(int argc, char* argv[]){ //char** argv
    printf("USAGE...");

    char* flag = argv[1];

    if(strcmp(flag,"-1") == 0) ex1();
    else if(strcmp(flag,"-2") == 0) ex2();
    else if(strcmp(flag,"-3") == 0) ex3(argc-2, argv);
    else if(strcmp(flag,"-4") == 0) ex4(argv[2]);
    //else if(strcmp(flag,"-5") == 0) ex5(atoi(argv[2]));
    //else if(strcmp(flag,"-6") == 0) ex6(atoi(argv[2]));
    else {
        printf("Invalid flag.\n");
        return 1;
    }

    return 0;
}