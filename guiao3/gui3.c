#include <sys/wait.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h> //system()

//1. Implemente um programa que execute o comando ls -l. Note que no caso da execução ser bem
//sucedida, mais nenhuma outra instrução é executada do programa original.
int ex1(){
    int exec_ret1 = execl("/bin/ls", "ls", "-l", NULL);
    //  caso o path seja correto, o codigo é substituido e o printf não é executado.
    //  caso o path seja incorreto "/bin/sudhvfsv", res tem valor de retorno e o printf é executado.
    int exec_ret2 = execlp("ls", "ls", "-l", NULL);

    char* exec_args[] = {"/bin/ls", "-l", NULL};
    int exec_ret3 = execv("/bin/ls", exec_args); 
    int exec_ret4 = execvp("ls", exec_args); 

    printf("Exec reached return: %d\n", exec_ret1);
    return 0;
}

//2. Implemente um programa semelhante ao anterior que execute o mesmo comando mas agora no contexto
//de um processo filho.
int ex2(){
    int exec_ret;

    int resf = fork();
    if(resf == 0){
        sleep(2);

        exec_ret = execl("/bin/ls", "ls", "-l", NULL); 
        printf("[FILHO %d] Exec went wrong: %d\n", getpid(), exec_ret);

        _exit(1); //importante pq caso exec corra mal, processo é terminado either way (com status 1 em vez de 0)
    } else {
        pid_t terminated_pid = wait(NULL); //forcar execucao sequencial, ao inves de paralela/concorrente
        
        printf("[PAI   %d] Child process exited normally\n", terminated_pid);
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

            int exec_ret = execlp(argv[i+2], argv[i+2], NULL);
            printf("[FILHO %d] Exec went wrong: %d\n", getpid(), exec_ret);

            _exit(1);
        }
    }

    for(int i = 0; i < args; i++){
        int status;
        pid_t terminated_pid = wait(&status);
        //-1 for error or pid

        if(WIFEXITED(status))
            //WEXITSTATUS(status) = 1 (_exit(1)), se exec went wrong, otherwise 0
            printf("[PAI   %d] Child process exited normally with status: %d\n", terminated_pid, WEXITSTATUS(status));
        else //nunca vai chegar aqui por causa do _exit (independentemente do valor do _exit)
            printf("[PAI   %d] Child process exited abnormally...\n", i);
    }

    return 0;
}

//41. system example
int ex4_system(){ 

    int system_res = system("ls -la");
    //SYSTEM()
    //  executes the command by passing it to the system shell
    //  internally calls fork() and exec() functions to create a new process and execute the command in that process.
    //  0 -> command completed successfully; non-zero -> there was an error.
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
    
    char* string = strtok(command, " ");
    while (string != NULL) {
        exec_args[i] = string;
        printf("%s\n", exec_args[i]);
        string = strtok(NULL, " ");
        i++;
    }
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

// EXERCICIOS ADICIONAIS
//1
//readln do stdin
//passar ao mysystem
//mysystem(string command) -> sem wait e a correr no filho, quando terminar to terminal

//2 execucao sequencial dentro de uma paralela
// C
// | -> a (se return nao for pretendido:) -> a -> a
// | -> b
// | -> c


int main(int argc, char* argv[]){ //char** argv
    printf("USAGE...\n");

    char* flag = argv[1];

    if(strcmp(flag,"-1") == 0) ex1();
    else if(strcmp(flag,"-2") == 0) ex2();
    else if(strcmp(flag,"-3") == 0) ex3(argc-2, argv); //./gui3 -3 ls ps
    else if(strcmp(flag,"-41") == 0) ex4_system(); //system function with 
    else if(strcmp(flag,"-42") == 0) { //mysystem with predefined commands 
        char command1[] = "ls -l -a -h"; //ret: 0
        char command2[] = "sleep 3";     //ret: 0
        char command3[] = "lss";        //ret: 255
        //_exit(-1) is being translated to 255 in WEXITSTATUS() because of the way that negative
        // exit statuses are represented in the status value returned by wait(), 
        // and the way that WEXITSTATUS() extracts the exit status from this value.
        //exit status is stored as an unsigned byte in the low-order 8 bits of the status value.
        //(reaches the exit)
        char command4[] = "ls -j";       //ret: 2 (invalid flag)
        //ls command is printing an error message and exiting with a status code of 2
        // when it encounters the -j option (doesnt reach the _exit)

        printf("A executar mysystem para \"%s\"...\n", command1);
        int ret = ex4(command1);
        printf("ret: %d\n\n", ret);

        printf("A executar mysystem para \"%s\"...\n", command2);
        ret = ex4(command2);
        printf("ret: %d\n\n", ret);

        printf("A executar mysystem para \"%s\"...\n", command3);
        ret = ex4(command3);
        printf("ret: %d\n\n", ret);

        printf("A executar mysystem para \"%s\"...\n", command4);
        ret = ex4(command4);
        printf("ret: %d\n\n", ret);
    }
    else if(strcmp(flag,"-43") == 0){
        printf("A executar mysystem para \"%s\"...\n", argv[2]);
        int ret = ex4(argv[2]); //agrv[2] as string: ./gui3 -43 "ls -l -a" or "pss"
        printf("ret: %d\n", ret);
    }
    else {
        printf("Invalid flag.\n");
        return 1;
    }

    return 0;
}