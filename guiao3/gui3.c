#include <sys/wait.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h> //system()

//1. Implemente um programa que execute o comando ls -l. Note que no caso da execução ser bem
//sucedida, mais nenhuma outra instrução é executada do programa original.
int ex1(){
    printf("Exec Starting...\n");

    int exec_ret1 = execl("/bin/ls", "ls", "-l", NULL);
    //caso o path seja correto, o codigo é substituido e o printf não é executado.
    //caso o path seja incorreto "/bin/sudhvfsv", exec_ret1 tem valor de retorno e o printf final é executado.
    
    //int exec_ret2 = execlp("ls", "ls", "-l", NULL);
    //char* exec_args[] = {"/bin/ls", "-l", NULL};
    //int exec_ret3 = execv("/bin/ls", exec_args); 
    //int exec_ret4 = execvp("ls", exec_args); 

    printf("Exec reached return: %d\n", exec_ret1);
    perror("Reached Return");
    return 0;
}

//2. Implemente um programa semelhante ao anterior que execute o mesmo comando mas agora no contexto
//de um processo filho.
int ex2(){
    int exec_ret;
    //char* exec_args[] = {"/bin/ls", "ls", NULL};

    int resf = fork();
    if(resf == 0){
        sleep(2);

        exec_ret = execl("/bin/ls", "ls", "-l", NULL); 
        //exec_ret = execvp("ls", exec_args);
        printf("[FILHO %d] After Exec | Exec went wrong: %d\n", getpid(), exec_ret);

        _exit(1); //importante pq caso exec corra mal, processo é terminado either way (com status 1 em vez de 0)
    } else {
        //pid_t terminated_pid = wait(NULL); 
        //printf("[PAI   %d] Child process exited normally\n", terminated_pid);

        int status;
        int terminated_pid = wait(&status); //forcar execucao sequencial, ao inves de paralela/concorrente
        //-1 for error or pid

        if(WIFEXITED(status))
            //WEXITSTATUS(status) = 1 (_exit(1)), se exec went wrong, otherwise 0
            printf("[PAI   %d] Child process exited normally with status: %d\n", terminated_pid, WEXITSTATUS(status));
        else //nunca vai chegar aqui por causa do _exit (independentemente do valor do _exit)
            printf("[PAI   %d] Child process exited abnormally...\n", terminated_pid);
    }

    return 0;
}

//3. Implemente um programa que execute concorrentemente uma lista de executáveis especificados como
//argumentos da linha de comando. Considere os executáveis sem quaisquer argumentos próprios. O
//programa deverá esperar pelo fim da execução de todos processos por si criados.
int ex3(int args, char* argv[]){

    for(int i = 0; i < args; i++){
        int resf = fork();
        if(resf == 0){
            sleep(1);
            
            printf("[FILHO %d] Started | going to execute command %s]\n", getpid(), argv[i+2]);
            int exec_ret = execlp(argv[i+2], argv[i+2], NULL);
            printf("[FILHO %d] After Exec | Exec went wrong: %d\n", getpid(), exec_ret);

            perror("Reached Return");
            _exit(1);
        }
    }

    for(int i = 0; i < args; i++){
        int status;
        pid_t terminated_pid = wait(&status);

        if(WIFEXITED(status))
            printf("[PAI   %d] Child process exited normally with status: %d\n", terminated_pid, WEXITSTATUS(status));
        else
            printf("[PAI   %d] Child process exited abnormally...\n", terminated_pid);
    }

    return 0;
}

//41. system example
int ex4_system(){ 

    int system_res = system("ls -la");
    //SYSTEM()
    //  executes the command by passing it to the system shell
    //  internally calls fork(2) and execl() to create a new process and execute the command in that process shell.
    //      Command is NULL: a nonzero value if a shell is available, or 0 if no shell is available.
    //      Child process could not be created or its status could not be retrieved: -1 and errno is set to indicate the error.
    //      Shell could not be executed in child: child shell terminated by calling _exit(2) with the status 127.
    //      All system calls succeed: is the termination status of the child shell used to execute command.
    //EXECVP()
    //  replaces the current process image with a new process image.
    //  if successful, the current program will no longer be running.
    
    printf("System reached return: %d\n", system_res);
    return 0;
}


//4. Implemente uma versão simplificada da função system(). Ao contrário da função original, não tente
//suportar qualquer tipo de redireccionamento, ou composição/encadeamento de programas executáveis.
//O único argumento deverá ser uma string que especifica um programa executável e uma eventual lista de
//argumentos. Procure que o comportamento e valor de retorno da sua função sejam compatíveis com a
//original.
int ex4(char* command) {
    char* exec_args[32]; // maximum number of arguments
    int i = 0;
    int retf;
    int status;
    int res;
    
    char* string = strtok(command, " ");   //char* strtok (char* str, const char* delim)
    while (string != NULL) {
        exec_args[i] = string;
        printf("%s ", exec_args[i]);
        string = strtok(NULL, " ");
        i++;
    }
    printf("\n");
    exec_args[i] = NULL; //set the last element to NULL
    
    //Sem fork o programa principal era trocado
    retf = fork();
    if(retf == 0){
        int exec_ret = execvp(exec_args[0], exec_args);
        printf("%d\n", exec_ret);

        _exit(exec_ret); //exec_ret = -1 MAS res = 255

    } else {
        if(retf != -1){
            waitpid(retf, &status, 0);
            if(WIFEXITED(status))
                res = WEXITSTATUS(status);
            else
                res = -1;
        } else res = -1;
    }
    
    return res;
}

//4. (versao alternativa) Igual ao exercicio 3 mas corre varios processos do mesmo programa com flags diferentes (em vez de varios progs dif)
//Assume-se, como no ex3, argv[] = ./gui3 -X "ls -l -a"
//não testada
int ex4_alt(int args, char* argv[]){

    for(int i = 0; i < args; i++){
        int resf = fork();
        if(resf == 0){
            sleep(1);
            
            printf("[FILHO %d] Started | going to execute command %s %s]\n", getpid(), argv[2], argv[2+i]);
            int exec_ret = execlp(argv[2], argv[2], argv[i+2], NULL);
            //Atenção ao usar o execvp: exec_ret = execvp (argv[i+2], &argv[i+2]); (para char** argv?)
            printf("[FILHO %d] After Exec | Exec went wrong: %d\n", getpid(), exec_ret);

            perror("Reached Return");
            _exit(exec_ret);
        }
    }

    for(int i = 0; i < args; i++){
        int status;
        pid_t terminated_pid = wait(&status);

        if(WIFEXITED(status))
            printf("[PAI   %d] Child process exited normally with status: %d\n", terminated_pid, WEXITSTATUS(status));
        else
            printf("[PAI   %d] Child process exited abnormally...\n", terminated_pid);
    }

    return 0;
}

// EXERCICIOS ADICIONAIS
//1
//readln do stdin
//passar ao mysystem
//mysystem(string command) -> sem wait e a correr no filho, quando terminar to terminal

//1. Implemente um interpretador de comandos muito simples ainda que inspirado na bash. O interpretador
//deverá executar comandos especificados numa linha de texto introduzida pelo utilizador. Os comandos
//são compostos pelo nome do programa a executar e uma eventual lista de argumentos. Os comandos
//podem ainda executar em primeiro plano, ou em pano de fundo, caso o utilizador termine a linha com &.
//O interpretador deverá terminar a sua execução quando o utilizador invocar o comando interno exit ou
//quando assinalar o fim de ficheiro (Control-D no início de uma linha em sistemas baseados em Unix).
int ex5(){
    printf("Not yet implemented...\n");
    return 1;
}

//2 execucao sequencial dentro de uma paralela
// C
// | -> a (se return nao for pretendido:) -> a -> a
// | -> b
// | -> c

//2. Implemente um programa controlador que execute concorrentemente um conjunto de programas
//especificados como argumento da sua linha de comando. O controlador deverá re-executar cada
//programa enquanto não terminar com código de saída nulo. No final da sua execução, o controlador
//deverá imprimir o número de vezes que cada programa foi executado. Considere que os programas são
//especificados sem qualquer argumento.
int ex6(){
    printf("Not yet implemented...\n");
    return 1;
}

int main(int argc, char* argv[]){ //char** argv
    printf(
"\n---------------------------------\n\
Welcome to the program!\n\
Please select an option below:\n\n\
Flag  Function\n\
----  --------\n\
-1    Correctly executes \'ls -l\' with execl, execlp. execv, execvp.\n\
-2    Same as above, but with execl and in the context of a child sequential process.\n\
-3    Creates child parallel processes and executes a list of executables specified as command line arguments.\n\
      The program waits for the end of execution of all processes created by it.\n\
-41   Example of the execution of the system() call of \'ls -l\'.\n\
-42   Simulate system() with own function by executing a set of pre-defined commands.\n\
-43   Simulate system() with own function by executing commands and arguments specified as a command line string argument.\n\
-5    (not yet) This code implements a very simple command interpreter inspired by bash.\n\
      The interpreter executes commands specified in a line of text entered by the user.\n\
      The commands consist of the name of the program to be executed and an optional list of arguments.\n\
      The commands can also be executed in the foreground or background if the user terminates the line with &.\n\
      The interpreter should terminate its execution when the user invokes the internal exit command or \n\
      when it signals the end of file (Control-D at the beginning of a line in Unix-based systems).\n\
-6    (not yet) ...\n\
\n\
USAGE\n\
  ./gui3 -1\n\
  ./gui3 -2\n\
  ./gui3 -3 [command1] [command2] [commandN] (./gui3 -3 ls ps)\n\
  ./gui3 -41\n\
  ./gui3 -42\n\
  ./gui3 -43 \"[command]\" (./gui3 -43 \"ls -l -a\")\n\
---------------------------------\n\n");


    char* flag = argv[1];

    if(strcmp(flag,"-1") == 0) ex1();
    else if(strcmp(flag,"-2") == 0) ex2();
    else if(strcmp(flag,"-3") == 0) ex3(argc-2, argv);
    else if(strcmp(flag,"-41") == 0) ex4_system();
    else if(strcmp(flag,"-42") == 0) {
        char command1[] = "ls -l -a -h"; //ret: 0
        char command2[] = "sleep 3";     //ret: 0
        char command3[] = "lss";         //ret: 255
        //_exit(-1) is being translated to 255 in WEXITSTATUS() because of the way that negative
        // exit statuses are represented in the status value returned by wait(), 
        // and the way that WEXITSTATUS() extracts the exit status from this value.
        //exit status is stored as an unsigned byte in the low-order 8 bits of the status value.
        //(reaches the exit)
        char command4[] = "ls -j";       //ret: 2 (invalid flag)
        //ls command is printing an error message and exiting with a status code of 2
        // when it encounters the -j option (doesnt reach the _exit)

        printf("[MAIN] A executar mysystem para \"%s\"...\n", command1);
        int ret = ex4(command1);
        printf("[MAIN] Return Value: %d\n----------------------------------------\n\n", ret);

        printf("[MAIN] A executar mysystem para \"%s\"...\n", command2);
        ret = ex4(command2);
        printf("[MAIN] Return Value: %d\n----------------------------------------\n\n", ret);

        printf("[MAIN] A executar mysystem para \"%s\"...\n", command3);
        ret = ex4(command3);
        printf("[MAIN] Return Value: %d\n----------------------------------------\n\n", ret);

        printf("[MAIN] A executar mysystem para \"%s\"...\n", command4);
        ret = ex4(command4);
        printf("[MAIN] Return Value: %d\n----------------------------------------\n\n", ret);
    }
    else if(strcmp(flag,"-43") == 0){
        printf("[MAIN] A executar mysystem para \"%s\"...\n", argv[2]);
        int ret = ex4(argv[2]); //agrv[2] as string: ./gui3 -43 "ls -l -a" or "pss"
        printf("[MAIN] Return Value: %d\n", ret);
    }
    else if(strcmp(flag,"-5") == 0) ex5();
    else if(strcmp(flag,"-6") == 0) ex6();
    else {
        printf("Invalid flag.\n");
        return 1;
    }

    return 0;
}