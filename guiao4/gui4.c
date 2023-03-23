#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdio.h>


//1. Escreva um programa que crie um pipe anónimo e de seguida crie um processo filho. Experimente o pai
//enviar um inteiro atraves do descritor de escrita do pipe, e o filho receber um inteiro a partir do respetivo
//descritor de leitura.
void ex1(){
    //primeiro criar o pipe
    //e apenas depois fazer fork



}

//ex6
//exercicio da matriz
//criar varios processos filhos, cada um procurar numa linha e escrever no pipe em que linha e nº de vezes que encontrou na linha
//ler do pipe pelo pai e apresentar
//podemos enviar uma struct pelo pipe, com a info de cima. (make sure que a struct seja menor que o pipe buff!)

//ex3
//1. usar um for para varios write mas pai só le um pouquinho (uma vez). -> o pai le apenas o zero
//2. ao escrever um escreveu continuamente e quando chega a um valor, enche o buffer. tamanho do buffer corresponde a sizeof(int) * numero maximo escrito

//ex5
//nao queremos ler so um int, ler ate que seja necessario. vemos que é parecido com os ficheiros (gui1).
//recorremos a um while((res = read...) > 0)
//filho escreve ate tam max do buffer, ate o pai decidir consumir e ai escrevem e leem concorrentemente
//vamos sempre ler ate end of file, 

void ex2_filhoToPai(){
    int p[2];
    pipe(p);

    int resf = fork();
    if (resf == 0){
        close(p[0]); //impedir de ler

        int test = 444;
        printf("sou o filho e vou escrever...\n");
        write(p[1], &test, sizeof(int));
        printf("sou o filho e escrevi %d...\n", test);

        close(p[1]);
        _exit(0);
    } else {

        close(p[1]); //impedir de escrever
        
        int recebido;
        printf("sou o pai e vou ler...\n");
        read(p[0], &recebido, sizeof(int));
        printf("sou o pai e recebi %d...\n", recebido);

        close(p[0]); //impedir de escrever
        wait(NULL); //nao necessario, apenas para demonstracao
    }

}

void ex1_paiToFilho(){
    int p[2];
    pipe(p);

    int res = fork();
    if(res == 0){
        close(p[1]); //sempre que nao devermos usar algo fazer close
        //close do fd de escrita
        
        int recebido;
        read(p[0], &recebido, sizeof(int)); //ler do fd 0 e guardar no recebido um int
        printf("sou o filho e recebi %d\n", recebido);

        close(p[0]);
        _exit(0);

    } else {

        //o pai ao escrever, tem que fechar a leitura
        close(p[0]);

        int i = 23;
        printf("sou o pai e vou escrever.\n");
        sleep(10);
        write(p[1], &i, sizeof(int));
        printf("escrevi.\n");


        close(p[1]);
        wait(NULL);
    }
}


int main(int argc, char* argv[]){ //char** argv
    printf("USAGE...\n");

    char* flag = argv[1];
    
    if(strcmp(flag,"-1") == 0) ex1_paiToFilho();
    //if(strcmp(flag,"-2") == 0) ex1_paiToFilho_withSleep();
    if(strcmp(flag,"-3") == 0) ex2_filhoToPai();
    if(strcmp(flag,"-3") == 0) ex6_MatrixSearch();
    else {
        printf("Invalid flag.\n");
        return 1;
    }

    return 0;
}