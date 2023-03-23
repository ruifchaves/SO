#include <sys/wait.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h> //system()

//Exercícios adicionais
//1
//readln do stdin
//passar ao mysystem
//mysystem(string command) -> sem wait e a correr no filho, quando terminar to terminal


//2 execucao sequencial dentro de uma paralela
// C
// | -> a (se return nao for pretendido:) -> a -> a
// | -> b
// | -> c




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
    int wait_ret;
    int res;
    
    char* string = strtok(command, " ");
    while (string != NULL) {
        exec_args[i] = string;
        string = strtok(NULL, " ");
        i++;
    }
    exec_args[i] = NULL; //set the last element to NULL
    
    //Sem fork o programa principal era trocado
    retf = fork();
    if(retf == 0){
        int exec_ret = execvp(exec_args[0], exec_args);
        _exit(exec_ret);
    } else {
        if(retf != -1){
            wait_ret = waitpid(retf, &status, 0);
            if(WIFEXITED(status))
                res = WEXITSTATUS(status);
            else
                res = -1;
        } else res = -1;
    }
    
    return res;
}

//system example
int ex4_system(){ 

    int res = system("ls -la");
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
            int exec_ret = execlp(argv[i+2], argv[i+1], NULL);
            printf("Correu mal... %d\n", exec_ret);
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

//2. Implemente um programa semelhante ao anterior que execute o mesmo comando mas agora no contexto
//de um processo filho.
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

    printf('Terminou... %d\n', res);

    return 0;
}

//1. Implemente um programa que execute o comando ls -l. Note que no caso da execuc¸ao ser bem ˜
//sucedida, mais nenhuma outra instruc¸ao˜ e executada do programa original.
int ex1(){
    int res = execl("/bin/ls", "ls", "-l", NULL); 
    //caso o path seja correto, o codigo é substituido e o printf não é executado.
    //caso o path seja incorreto "/bin/sudhvfsv", res tem valor de retorno e o printf é executado.

    printf('Terminou... %d\n', res);

    return 0;
}


int main(int argc, char* argv[]){ //char** argv
    printf("USAGE...");

    char* flag = argv[1];

    if(strcmp(flag,"-1") == 0) ex1();
    else if(strcmp(flag,"-2") == 0) ex2();
    else if(strcmp(flag,"-3") == 0) ex3(argc-2, argv);
    else if(strcmp(flag,"-41") == 0) ex4_system(); //system function with 
    else if(strcmp(flag,"-42") == 0) { //mysystem with predefined commands 
        char command1[] = "ls -l -a -h";
        char* command1 = "ls -l -a -h";
        char command2[] = "sleep 10";
        char command3[] = "ps";
        int ret;

        printf("A executar mysystem para \"%s\"...\n", command1);
        int ret = ex4(command1);
        printf("ret: %d\n", ret);

        printf("A executar mysystem para \"%s\"...\n", command2);
        ex4(command2);
        printf("A executar mysystem para \"%s\"...\n", command3);
        ex4(command3);
    }
    else if(strcmp(flag,"-43") == 0) ex4(argv[2]);      //agrv[2] as string: ./gui3 -43 "ls -l -a"
    else {
        printf("Invalid flag.\n");
        return 1;
    }

    return 0;
}