#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>

//1. Escreva trˆes programas que ir ̃ao ilustrar a operac ̧  ̃ao de pipes com nome. O primeiro cria um pipe com
//nome “fifo”. O segundo repete para este pipe todas as linhas de texto lidas a partir do seu standard input.
//Por sua vez, o terceiro programa repete para o seu standard output todas as linhas de texto lidas a partir
//deste mesmo pipe.
//Note que, ao contr ́ario dos pipes an  ́onimos, a abertura para escrita de um pipe com nome bloqueia at ́e
//que um processo o abra para leitura, e vice-versa.
int ex1(){ 
    //ao correr isto cria um ficheiro myfifo que corresponde a um buffer
    //programa cria o fifo e depois fecha
    //mas este ficheiro pode ser utilizado para comunicacao entre processos sem afiliacao ao processo que o criou


    //
    //myfifo - nome que queremos far ao pipe fifo
    int res = mkfifo("myfifo", 0640);
    if(res < 0){
        printf("Criei o pipe\n");
    }


    printf("PIPEBUF -> %d\n", PIPE_BUF);

    return 0;
}


int ex12(){ //vamos escrever no pipe
    //ao correr apenas isto antes do ex13 fica bloqueado no read
    int fd = open("myfifo", O_WRONLY);
    if(fd < 0){
        perror("erro no open\n");
    }

    char buffer[50];
    int res;
    printf("Abri o pipe!\n"); //so aparece isto quando o fd de leitura abrir porque bloqueia no open
    while((res = read(0, &buffer, 50)) > 0){

        write(fd, &buffer, res);
        printf("Escrevi %s\n", buffer);
    }

    close(fd);
    printf("terminei\n");
    return 0;
}

//ao abrir varios deste aka ter multiplos escritores temos que ter o cuidado
//de que o que escrevemos em cada um é menor que o PIPE BUFF para ter a certeza
//de que a leitura vai ser atómica
int ex13(){ //vamos ler do pipe e escrever no terminal
    //ao fechar esta funcao, ao fechar o fd de escrita o ex12 vai morrer, porque chegou ao EOF
    
    int fd = open("myfifo", O_RDONLY);
    //printf("\n");

    //if(fd < 0){
    //    perror("erro no open\n");
    //}

    char buffer[50];
    int res;
    while((res = read(fd, &buffer, 50)) > 0){ //agora do fd e nao do stdin
        //printf("L %s\n", buffer);
        write(1, &buffer, res);
    }

    close(fd);
    printf("terminei\n");
    return 0;
}

int ex14(){ //vai remover o buffer myfifo
    unlink("myfifo");

    return 0;
}


//ao ter multiplos leitores nao temos garantia de quem vai ler o que
//ao ser lido vai ser removido do buffer
int ex21(){
    int pipe_fd;
    printf("### SERVIDOR STARTING ###\n");

    printf("### CREATING FILE ###\n");

    printf("### CREATING PIPE ###\n");
    char pipename[50] = "piping";
    int pipe_create = mkfifo(pipename, 0640);    
    if(pipe_create < 0){
        perror("Created pipe...\n");
    }

    printf("### CREATING LOG FILE ###\n");
    char filename[50] = "log.txt";
    int fd = open(filename, O_CREAT | O_WRONLY, 0640);    
    if(fd < 0){
        perror("Error creating log file...\n");
        return -1;
    }

    //englobal num while o open do pipe e 
    //while(1){open; read}

    //ou entao antes de abrir e de forma a nao ter o ciclo while, enganar correndo um open(write) antes do open
    while(1){
        pipe_fd = open(pipename, O_WRONLY);
        if(fd < 0){
            perror("Error opening pipe\n");
        }

        printf("### READY FOR LOG ###\n");
        char buffer[50];
        while((int res = read(pipe_fd, &buffer, 50)) > 0){
            print("Reading from pipe: %s", buffer);
            write(fd, &buffer, res);
            printf("Logging: %s\n", buffer);
        }
    }

    close(pipe_fd);
    close(fd);
    unlink(pipename);
    return 0;
}




int ex22(char* input[]){
    printf("### CLIENT STARTING ###\n");
    printf("INPUT: %s\n", input);

    char pipename[50] = "piping";
    int pipe_fd = open(pipename, O_RDONLY);

    char buffer[50];
    write(pipe_fd, &buffer, sizeof(input));
    printf("Sending to pipe: %s\n", buffer);
    //while((int res = read(fd, &buffer, 50)) > 0){
    //    write(1, &buffer, res);
    //}

    close(pipe_fd);
    printf("terminei\n");
    return 0;
}

int main(int agrc, char* argv[]){
    //demonstracao tendo dois terminais abertos
    printf("USAGE...\n\n");

    char* flag = argv[1];
    
    if(strcmp(flag,"-11") == 0) ex1();
    else if(strcmp(flag,"-12") == 0) ex12();
    else if(strcmp(flag,"-13") == 0) ex13();
    else if(strcmp(flag,"-14") == 0) ex14();
    else if(strcmp(flag,"-21") == 0) ex21();
    else if(strcmp(flag,"-22") == 0) ex22(argv[2]);
    else {
        printf("Invalid flag.\n");
        return 1;
    }

    return 0;
}