#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#define RANDOM_MAX (int)10e6

//1. Escreva um programa que crie um pipe anónimo e de seguida crie um processo filho. Experimente o pai
//enviar um inteiro atraves do descritor de escrita do pipe, e o filho receber um inteiro a partir do respetivo
//descritor de leitura.
void ex1_paiToFilho(){
    int p[2];  //array que guarda os dois file descriptors do pipe, daí ser 2
    pipe(p);   //primeiro criar o pipe e apenas depois fazer fork

    int resf = fork();
    if(resf == 0){
        close(p[1]); //sempre que nao devermos usar algo fazer close. close do fd de escrita
        
        int recebido;
        printf("[FILHO] Vou ler...\n");
        read(p[0], &recebido, sizeof(int)); //ler do fd 0 e guardar no recebido um int
        printf("[FILHO] Recebi: %d\n", recebido);

        close(p[0]); //fechar extremidade de leitura (nao preciso mais dela)
        _exit(0); //exit sempre no filho

    } else {
        close(p[0]); //o pai que vai apenas escrever, tem que fechar a leitura

        int intToWrite = 23;
        printf("[PAI]   Vou escrever\n");
        write(p[1], &intToWrite, sizeof(int));
        printf("[PAI]   Escrevi: %d\n", intToWrite);

        close(p[1]);
        wait(NULL);
    }
}


//2. Experimente de seguida provocar um atraso antes do pai enviar o inteiro (p. ex., sleep(5)). Note
//agora que a leitura do filho bloqueia enquanto o pai nao realizar a operação de escrita no pipe.
void ex2_paiToFilho_withSleep(){
    int p[2];
    pipe(p);

    int resf = fork();
    if(resf == 0){
        close(p[1]);
        
        int recebido;
        printf("[FILHO] Vou ler...\n");
        read(p[0], &recebido, sizeof(int));
        printf("[FILHO] Recebi: %d\n", recebido);

        close(p[0]);
        _exit(0);

    } else {
        close(p[0]);

        int intToWrite = 23;
        printf("[PAI]   Vou escrever...\n");
        sleep(10);
        write(p[1], &intToWrite, sizeof(int));
        printf("[PAI]   Escrevi: %d\n", intToWrite);

        close(p[1]);
        wait(NULL);
    }
}

//3. Experimente agora inverter os papeis de modo a informação ser transmitida do filho para o pai.
void ex3_filhoToPai(){
    int p[2];
    pipe(p);

    int resf = fork();
    if (resf == 0){
        close(p[0]); //impedir de ler

        int intToWrite = 444;
        printf("[FILHO] Vou escrever...\n");
        write(p[1], &intToWrite, sizeof(int));
        printf("[FILHO] Escrevi: %d\n", intToWrite);

        close(p[1]); //impedir de escrever
        _exit(0);

    } else {
        close(p[1]); //impedir de escrever
        
        int recebido;
        printf("[PAI]   Vou ler...\n");
        read(p[0], &recebido, sizeof(int));
        printf("[PAI]   Li: %d\n", recebido);

        close(p[0]); //impedir de ler
        wait(NULL); //nao necessario, apenas para demonstracao
    }
}

//2. 
//4. Experimente de seguida provocar um atraso antes do pai ler o inteiro.
void ex4_1(){
    int p[2];
    pipe(p);

    int resf = fork();
    if (resf == 0){
        close(p[0]); //impedir de ler

        int intToWrite = 444;
        printf("[FILHO] Vou escrever...\n");
        write(p[1], &intToWrite, sizeof(int));
        printf("[FILHO] Escrevi: %d\n", intToWrite);

        close(p[1]); //impedir de escrever
        _exit(0);

    } else {
        close(p[1]); //impedir de escrever
        
        sleep(2);
        int recebido;
        printf("[PAI]   Vou ler...\n");
        read(p[0], &recebido, sizeof(int));
        printf("[PAI]   Li: %d\n", recebido);

        close(p[0]); //impedir de ler
        wait(NULL); //nao necessario, apenas para demonstracao
    }
}

//4. Repita com uma sequencia de inteiros. Note agora que escrita do filho bloqueia enquanto o pai nao realizar a operação de leitura no pipe.
void ex4_2(){
    int p[2];
    pipe(p);

    //filho escreve continuamente até um valor, enchendo o buffer.
    //tamanho do buffer corresponde a sizeof(int) * numero maximo escrito
    for(int i = 0; i < 1000; i++){ //for para varios write mas pai só le um pouquinho (uma vez). -> o pai le apenas o zero
        int resf = fork();
        if(resf == 0){
            close(p[0]); //impedir de ler
            
            int written_int = 55;
            printf("[FILHO %d] Vou escrever...\n", i);
            write(p[1], &written_int, sizeof(int));
            printf("[FILHO %d] Escrevi: %d\n", i, written_int);

            close(p[1]); //impedir de escrever
            _exit(0);
        } else {
            close(p[1]); //impedir de escrever

            sleep(4);
            int recebido;
            printf("[PAI] Vou ler...\n");
            read(p[0], &recebido, sizeof(int));
            printf("[PAI] Li: %d\n", recebido);

            close(p[0]); //impedir de ler
        }
    }
}

//5. Modifique o programa anterior de modo a leitura do pipe ser realizada enquanto não for detetada a
//situação de /end of file/ no descritor respetivo. Repare que esta situação acontece apenas quando nenhum
//processo – neste caso, pai e filho – tem aberto o descritor de escrita do pipe.
void ex5(){
    int p[2];
    pipe(p);

    //nao queremos ler so um int, ler ate que seja necessario. vemos que é parecido com os ficheiros (gui1).
    //filho escreve ate tam max do buffer, ate o pai decidir consumir e ai escrevem e leem concorrentemente
    //vamos sempre ler ate end of file,  um while((res = read...) > 0)

    for(int i = 0; i < 100000; i++){
        int resf = fork();
        if(resf == 0){
            close(p[0]); //impedir de ler

            int written_int = 555;
            //printf("[FILHO %d] Vou escrever...\n", i);
            write(p[1], &written_int, sizeof(int));
            printf("[FILHO %d] Escrevi: %d\n", i, written_int);

            close(p[1]); //impedir de escrever
            _exit(0);
        } //else {
            //close(p[1]); //impedir de escrever
           
            //int status;
            //wait(&status);
            
            //sleep(4);
            //int read_int, res;
            //printf("[PAI    ] Vou ler...\n");
            //while((res = read(p[0], &read_int, sizeof(int))) > 0){
            //    printf("[PAI    ] Li: %d\n", read_int);
            //}

            //read(p[0], &read_int, sizeof(int));
            //printf("[PAI    ] Li: %d\n", read_int);
            //close(p[0]); //impedir de ler
        //}
    }

    sleep(5);
    int read_int, res;
    while((res = read(p[0], &read_int, sizeof(int))) > 0){
        printf("[PAI    ] Li: %d\n", read_int);
    }
    
}


//ex6
//exercicio da matriz
//criar varios processos filhos, cada um procurar numa linha e escrever no pipe em que linha e nº de vezes que encontrou na linha
//ler do pipe pelo pai e apresentar
//podemos enviar uma struct pelo pipe, com a info de cima. (make sure que a struct seja menor que o pipe buff!)

//6. Pretende-se determinar todas as ocorrencias de um determinado número inteiro nas linhas duma matriz de
//números inteiros, em que o número de colunas é muito maior do que o número de linhas. Implemente, utili-
//zando processos e pipes, uma função que devolva num vetor todas as ocorrências encontradas. A matriz
//inicial, o valor a procurar e o vetor onde guardar os resultados devem ser fornecidos como parâmetros.

typedef struct Info {
    int row;
    int nOcurr;
} Info;

//Auxiliary function: Generates a 'rows' x 'cols' matrix of random integers  
int** genMatrix(int rows, int cols){
    int** matrix = malloc(sizeof(int*) * rows);

    for(int i = 0; i < rows; i++){
        matrix[i] = malloc(sizeof(int) * cols);
        for(int j = 0; j < cols; j++)
            matrix[i][j] = rand() % RANDOM_MAX;
    }

    return matrix;
}

void ex6(int** matrix, int rows, int cols, int target, int* vetor){
    srand(time(NULL));

    int p[2];
    pipe[p];

    for //rows
        //criar fork responsavel pela linha i

        Ninfo res;

    write(p[1], &res, sizeof(Ninfo));

        //close and exit
        close()
        _exit(0)


    close(p[1]);

    Ninfo res;
    while (read(p[0], &res, sizeof(Ninfo) > 0))
        result[res, line_nr]=res.ocur_nr;


    //wait for all processes

}

int main(int argc, char* argv[]){
    printf(
"\n---------------------------------\n\
Welcome to the program!\n\
Please select an option below:\n\n\
Flag  Function\n\
----  --------\n\
-1    Create an anonymous pipe and a child process.\n\
      The father sends int through writing descriptor of the pipe,\n\
      and the child receives int from the respective reading descriptor.\n\
-2    Delay before the father sends int (e.g., sleep(5)).\n\
      Now note that the child's read blocks until the father performs the write operation on the pipe.\n\
-3    Invert the roles so that the information is transmitted from the child to the father.\n\
-41   Delay before the father reads the integer.\n\
-42   Repeat with a sequence of integers. Now note that the child's write blocks until the father\n\
      performs the read operation on the pipe.\n\
-5    Modify the previous program so that the reading of the pipe is performed while the end-of-file\n\
      situation is not detected in the respective descriptor. Note that this situation occurs only when\n\
      no process - in this case, father and child - has opened the writing descriptor of the pipe.\n\
-6    Using processes and pipes, searches for a given integer in a matrix of integers with a large number \n\
      of columns and few rows. The initial matrix, the value to search for, and the vector to store the \n\
      results should be provided as parameters.\n\
\n\
USAGE\n\
  ./gui4 [flag]\n\
  ./gui4 -6 [targer_number]\n\
---------------------------------\n\n");

    char* flag = argv[1];
    
    if(strcmp(flag,"-1") == 0) ex1_paiToFilho();
    else if(strcmp(flag,"-2") == 0) ex2_paiToFilho_withSleep();
    else if(strcmp(flag,"-3") == 0) ex3_filhoToPai();
    else if(strcmp(flag,"-41") == 0) ex4_1();
    else if(strcmp(flag,"-42") == 0) ex4_2();
    else if(strcmp(flag,"-5") == 0) ex5();
    else if(strcmp(flag,"-6") == 0) {
        int rows = 20;
        int columns = 1e6;
        int** matrix = genMatrix(rows, columns);

        if (argc < 3) {
            printf("Missing target number...\n")
            return 1;
        } else if (argc == 3) printf("Target number: %d\n")
        int target = ;
        int* vector = malloc(sizeof(int) * rows);
        
        ex6(matrix, rows, columns, target, vector);




    }
    else {
        printf("Invalid flag.\n");
        return 1;
    }

    return 0;
}
