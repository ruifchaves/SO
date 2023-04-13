#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>


//1. Escreva um programa que redirecione o descritor associado ao seu standard input para o ficheiro /etc/passwd,
//e o standard output e error respetivamente para saida.txt e erros.txt. Imediatamente antes de o
//programa terminar, o mesmo deve imprimir no terminal a mensagem ”terminei”.
int ex1(){
    int saida = open("saida.txt", O_CREAT | O_TRUNC | O_WRONLY, 0660);
    if (saida < 0){
        write(2, "Erro no open saída\n", 20);
        return -1;
    }

    int erros = open("erros.txt", O_CREAT | O_TRUNC | O_WRONLY, 0660);
    if (erros < 0){
        write(2, "Erro no open erros\n", 20);
        return -1;
    }

    int passwd = open("/etc/passwd", O_RDONLY);
    if (passwd < 0){
        write(2, "Erro no open passwd\n", 21);
        return -1;
    }

    int backup = dup(0);

    dup2(passwd, 0); //afinal o 0 já nao vem do input mas do /etc/passwd
    dup2(saida, 1);  //de igual forma, o output passa a ser o ficheiro saida.txt
    dup2(erros, 2);
    close(passwd);
    close(saida);
    close(erros);

    //ler do stdin ate EOF e escrever no stdout
    char buffer[20];
    int res;
    while ((res = read(0, &buffer, sizeof(buffer))) > 0){
        write(1, &buffer, res);
        write(2, &buffer, res);
    }
    //apos correr isto o conteudo do erros.txt e do saida.txt vai ser o mesmo (diff saida.txt erros.txt)

    dup2(backup, 1);
    close(backup);
    printf("Terminei.\n");

    return 0;
}


//2. Modifique o programa anterior de modo a que, depois de realizar os redirecionamentos, seja criado um
//novo processo que realize operações de leitura e escrita. Observe o conteúdo dos ficheiros. Repare que o
//processo filho “nasce” com as mesmas associações de descritores de ficheiros do processo pai.
int ex2(){
    int saida = open("saida.txt", O_CREAT | O_TRUNC | O_WRONLY, 0660);
    if (saida < 0){
        write(2, "Erro no open saída\n", 20);
        return -1;
    }

    int erros = open("erros.txt", O_CREAT | O_TRUNC | O_WRONLY, 0660);
    if (erros < 0){
        write(2, "Erro no open erros\n", 20);
        return -1;
    }

    int passwd = open("/etc/passwd", O_RDONLY);
    if (passwd < 0){
        write(2, "Erro no open passwd\n", 21);
        return -1;
    }

    int backup = dup(0);

    dup2(passwd, 0);
    dup2(saida, 1);
    dup2(erros, 2);
    close(passwd);
    close(saida);
    close(erros);

    int resf = fork(); //apos o fork os redirecionamentos são preservados
    if(resf == 0){
        char buffer[20];
        int res;
        while ((res = read(0, &buffer, sizeof(buffer))) > 0){
            write(1, &buffer, res);
            write(2, &buffer, res);
        }

        _exit(1); //se nao metermos isto, vai fazer com que o filho tambem dê print do terminei, porque executa o restante codigo
    }

    dup2(backup, 1);
    close(backup);
    printf("Terminei.\n");

    return 0;
}

//3. Modifique novamente o programa inicial de modo a que seja executado o comando wc, sem argumentos,
//depois do redirecionamento dos descritores de entrada e saída. Note que, mais uma vez, as associações -
//- e redirecionamentos – de descritores de ficheiros são preservados pela primitiva exec().
int ex3(){
    int saida = open("saida.txt", O_CREAT | O_TRUNC | O_WRONLY, 0660);
    if (saida < 0){
        write(2, "Erro no open saída\n", 20);
        return -1;
    }

    int erros = open("erros.txt", O_CREAT | O_TRUNC | O_WRONLY, 0660);
    if (erros < 0){
        write(2, "Erro no open erros\n", 20);
        return -1;
    }

    int passwd = open("/etc/passwd", O_RDONLY);
    if (passwd < 0){
        write(2, "Erro no open passwd\n", 21);
        return -1;
    }

    int backup = dup(0);

    dup2(passwd, 0);
    dup2(saida, 1);
    dup2(erros, 2);
    close(passwd);
    close(saida);
    close(erros);

    //wc < /etc/passwd > saida.txt
    //wc -> word count, terminar com ctrl+d (aka EOF)
    //tentar executar o wc com os redirecionamentos
    int exec_ret = execlp("wc", "wc", NULL);
    //o saida.txt vai agora ficar com o resultado textual do wc do conteudo do /etc/passwd

    //caso tenhamos o execlp no contexto de um processo filho, permite que continuemos a correr o resto do código no processo pai
    //e ter o print terminei p ex
    return 0;
}

//4. Escreva um programa que execute o comando wc num processo filho. O processo pai deve enviar ao
//filho através de um pipe anónimo uma sequência de linhas de texto introduzidas pelo utilizador no seu
//standard input. Recorra à técnica de redirecionamento de modo a associar o standard input do processo
//filho ao descritor de leitura do pipe anónimo criado pelo pai. Recorde a necessidade de fechar o(s)
//descritor(es) de escrita no pipe de modo a verificar-se a situação de end of file.
int ex4(){

    int p[2];
    pipe(p);
    int link;

    int resf = fork();
    if(resf == 0){
        close(p[1]); //fechar escrita

        dup2(p[0], 0);
        close(p[0]);

        //execlp(0, )


        close(p[0]);
        _exit(1)
    } else {

        close(p[0]); //fechar leitura
        link(p[1]);

        char buffer[40];
        while((res = read(0, &buffer, sizeof(buffer))) > 0){
            write(1, &buffer, sizeof(buffer));
        }

        close(p[1]);
    }

    return 0;
}

//5. Escreva um programa que emule o funcionamento do interpretador de comandos na execução encadeada
//de ls /etc | wc -l.
int ex5(){

    return 0;
}


//6. Escreva um programa que emule o funcionamento do interpretador de comandos na execução encadeada
//de grep -v ˆ# /etc/passwd | cut -f7 -d: | uniq | wc -l
int ex6(){

    return 0;
}

//EXERCÍCIOS ADICIONAIS
//1. Acrescente ao interpretador de comandos proposto no guiao 3 a possibilidade de redirecionar as entradas,
//saídas e erros dos comandos por ele executados. Considere os operadores de redirecionamento <, >, >>,
//2> e 2>>.
int ex7(){

    return 0;
}

//2. Acrescente ao interpretador de comandos proposto nos guioes anteriores a possibilidade de encadear as
//entradas e saídas de programas a executar atraves de pipes anónimos (operador |).
int ex8(){

    return 0;
}

int main(int argc, char* argv[]){
    if(argc == 1){
    printf(
"\n--------------------------------------------------------------------------------------------------------------------------\n\
Welcome to the program!\n\
Please select an option below:\n\n\
Flag  Function\n\
----  --------------------------------------------------------------------------------------------------------------------\n\
-1    Write a program that redirects the descriptor associated with its standard input to the file /etc/passwd, \n\
      and standard output and error to saida.txt and erros.txt, respectively.\n\
      Immediately before the program terminates, it should print the message \"terminei\" on the terminal.\n\
-2    Modify the previous program so that, after performing the redirections, a new process is created that performs \n\
      read and write operations. Observe the contents of the files. Note that the child process \"inherits\" the same \n\
      file descriptor associations from the parent process.\n\
-3    Modify the initial program again so that the wc command, without arguments, is executed after the input and output\n\
      descriptor redirections. Note that, once again, file descriptor associations and redirections are \n\
      preserved by the exec() primitive.\n\
-4    Write a program that executes the wc command in a child process. The parent process must send a sequence of text\n\
      lines entered by the user on its standard input to the child through an anonymous pipe. Use the redirection\n\
      technique to associate the standard input of the child process with the reading descriptor of the anonymous pipe\n\
      created by the parent. Remember to close the writing descriptor(s) on the pipe to verify the end-of-file condition.\n\
-5    Write a program that emulates the operation of the command interpreter in the chained execution of ls /etc | wc -l.\n\
-6    Write a program that emulates the operation of the command interpreter in the chained execution of\n\
      grep -v ^# /etc/passwd | cut -f7 -d: | uniq | wc -l.\n\
-7    Add to the command interpreter proposed in guide 3 the ability to redirect the inputs, outputs, and errors of the\n\
      commands it executes. Consider the redirection operators <, >, >>, 2>, and 2>>.\n\
-8    Add to the command interpreter proposed in previous guides the ability to chain the inputs and outputs of programs\n\
      to be executed through anonymous pipes (the | operator).\n\
\n\
USAGE\n\
  ./gui6 [flag]\n\
--------------------------------------------------------------------------------------------------------------------------\n\n");
    return -1;
    }
    char* flag = argv[1];
    
    if(strcmp(flag,"-1") == 0)      ex1();
    else if(strcmp(flag,"-2") == 0) ex2();
    else if(strcmp(flag,"-3") == 0) ex3();
    else if(strcmp(flag,"-4") == 0) ex4();
    else if(strcmp(flag,"-5") == 0) ex5();
    else if(strcmp(flag,"-6") == 0) ex6();
    else if(strcmp(flag,"-7") == 0) ex7();
    else if(strcmp(flag,"-8") == 0) ex8();
    else {
        printf("Invalid flag.\n");
        return 1;
    }

    return 0;
}