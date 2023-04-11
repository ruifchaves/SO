#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h> //system()


//1. Escreva três programas que irão ilustrar a operação de pipes com nome. O primeiro cria um pipe com nome “fifo”.
//O segundo repete para este pipe todas as linhas de texto lidas a partir do seu standard input.
//Por sua vez, o terceiro programa repete para o seu standard output todas as linhas de texto lidas a partir
//deste mesmo pipe. Note que, ao contrário dos pipes anónimos, a abertura para escrita de um pipe com nome bloqueia até
//que um processo o abra para leitura, e vice-versa.
int ex1(){ 
    //ao correr isto cria um ficheiro myfifo que corresponde a um buffer
    //programa cria o fifo e depois fecha
    //mas este ficheiro pode ser utilizado para comunicacao entre processos sem afiliacao ao processo que o criou

    //myfifo - nome que queremos far ao pipe fifo
    int res = mkfifo("myfifo", 0640);
    if(res < 0){
        perror("Error creating pipe");
        return -1;
    }

    printf("Pipe created.\n");
    printf("PIPEBUF Size: %d.\n", PIPE_BUF);
    return 0;
}


int ex12(){ //vamos escrever no pipe
    //ao correr apenas isto antes do ex13 fica bloqueado no read
    int fd = open("myfifo", O_WRONLY);
    if(fd < 0){
        perror("Error opening pipe");
        return -1;
    }

    printf("Pipe opened.\n"); //so aparece isto quando o fd de leitura abrir porque bloqueia no open
    char buffer[1024];
    int res;
    while((res = read(0, &buffer, 1024)) > 0){
        write(fd, buffer, res);
        
        //If the buffer array is not null-terminated, printf will continue printing characters from memory
        // until it happens to encounter a null character, which could result in unexpected behavior.
        //buffer[res] = '\0'; // Add null character
        //printf("Wrote: %s.\n", buffer);
    }

    close(fd);
    printf("Ended.\n");
    return 0;
}



//ao abrir varios deste aka ter multiplos escritores temos que ter o cuidado
//de que o que escrevemos em cada um é menor que o PIPE BUFF para ter a certeza de que a leitura vai ser atómica
int ex13(){ //vamos ler do pipe e escrever no terminal
    //ao fechar esta funcao, ao fechar o fd de escrita o ex12 vai morrer, porque chegou ao EOF
    
    int fd = open("myfifo", O_RDONLY);
    if(fd < 0){
        perror("Error opening pipe");
        return -1;
    }

    printf("Pipe opened.\n"); 
    char buffer[1024];
    int res;
    while((res = read(fd, &buffer, sizeof(buffer))) > 0){ //agora do fd e nao do stdin
        write(1, &buffer, res);
    }

    if (res == 0) {
        printf("Pipe closed.\n");
    } else {
        perror("Error reading from pipe");
    }

    close(fd);
    printf("Ended.\n");
    return 0;
}

int ex14(){ //vai remover o buffer myfifo

    unlink("myfifo");
    return 0;
}



//2. Escreva um programa “servidor”, que fique a correr em background, e acrescente a um ficheiro de “log”
//todas as mensagens que sejam enviadas por “clientes”. Escreva um programa cliente que envia para o
//servidor o seu argumento. Cliente e servidor devem comunicar via pipes com nome.

//ao ter multiplos leitores nao temos garantia de quem vai ler o que
//ao ser lido vai ser removido do buffer
//ler do pipe e escrever no ficheiro
int ex21(){
    int pipe_fd;

    printf("### SERVIDOR STARTING ###\n");

    printf("\n### CREATING PIPE ###\n");
    char pipename[50] = "piping";
    int pipe_create = mkfifo(pipename, 0640);
    if(pipe_create < 0){
        perror("    Error creating pipe");
        return -1;
    }
    printf("    Pipe created.\n");

    printf("\n### CREATING LOG FILE ###\n");
    char filename[50] = "log.txt";
    int fd = open(filename, O_CREAT | O_APPEND | O_WRONLY, 0640);
    if(fd < 0){
        perror("    Error opening log file");
        return -1;
    }
    printf("    Log file opened.\n");


    //englobar num while o open do pipe e 
    //while(1){open; read}
    //ou entao antes de abrir e de forma a nao ter o ciclo while, enganar correndo um open(write) antes do open
    while(1){
        pipe_fd = open(pipename, O_RDONLY);
        if(fd < 0){
            perror("    Error opening pipe");
            return -1;
        }
        printf("\n\n    Pipe opened.\n"); 

        printf("### READY FOR LOG ###\n");
        char buffer[50];
        int res;
        while((res = read(pipe_fd, &buffer, 50)) > 0){
            printf("    Read from pipe: %s\n", buffer);
            write(fd, &buffer, res);
            printf("    Logged to file: %s\n", buffer);

            char cwd[PATH_MAX];
            char command[PATH_MAX + 56]; //pathmax(cwd) + 7(rest) + 50(filename)
            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                snprintf(command, sizeof(command), "cat \"%s/%s\"", cwd, filename);
                printf("    Running: %s\n    ", command);
                system(command);
                printf("    ");
            }
        }
    }

    //com o while nao chega aqui
    close(pipe_fd);
    close(fd);
    unlink(pipename);
    printf("\n### ENDED ###\n");
    return 0;
}

//escrever argumento no pipe
int ex22(char* input){
    printf("### CLIENT STARTING ###\n");
    printf("    INPUT: %s.\n", input);

    char pipename[50] = "piping";
    int pipe_fd = open(pipename, O_WRONLY);
    if(pipe_fd < 0){
        perror("    Error opening pipe");
        return -1;
    }

    write(pipe_fd, input, strlen(input)); //sizeof(input) returns the size of the pointer, not the size of the actual string.
    printf("    Sent to pipe: %s.\n", input);
    //while((int res = read(fd, &buffer, 50)) > 0){
    //    write(1, &buffer, res);
    //}

    close(pipe_fd);
    printf("\n### ENDED ###\n");
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
-11   Creates a named pipe by calling mkfifo(). Also spits out the size of the pipe buffer.\n\
-12   Opens the named pipe for writing by calling open() with the O_WRONLY flag.\n\
      The function then reads input from the standard input and writes it to the named pipe using write().\n\
      The function continues reading from stdin and writing to the pipe until it encounters the end-of-file marker\n\
      (i.e. Ctrl+D or sends an EOF character). The function then closes the pipe using the close().\n\
-13   Opens the named pipe for reading by calling open() with the O_RDONLY flag.\n\
      The function then reads data from the named pipe using read() and writes it to the standard output using write().\n\
      The function continues reading and writing until it reaches EOF or an error occurs. Then the pipe is closed.\n\
-14   Removes the named pipe by calling unlink().\n\
-21   Implements a server program that listens for incoming messages from clients over a named pipe.\n\
      The function creates a named pipe by calling mkfifo(), opens a log file by calling open(), \n\
      and enters an infinite loop that waits for messages to arrive over the named pipe.\n\
      When a message arrives, the function writes it to the log file using write().\n\
-22   This function serves as a client process that sends a message to a server process through a named pipe.\n\
      The function takes an input string as an argument and opens the named pipe using open().\n\
      It then writes the input string to the pipe using write() before closing the pipe.\n\
\n\
USAGE\n\
  ./gui5 [flag]\n\
  ./gui5 -22 \"[message]\"\n\
--------------------------------------------------------------------------------------------------------------------------\n\n");
    return -1;
    }
    char* flag = argv[1];
    
    if(strcmp(flag,"-11") == 0) ex1();
    else if(strcmp(flag,"-12") == 0) ex12();
    else if(strcmp(flag,"-13") == 0) ex13();
    else if(strcmp(flag,"-14") == 0) ex14();
    else if(strcmp(flag,"-21") == 0) ex21();
    else if(strcmp(flag,"-22") == 0 && argc == 3) ex22(argv[2]);
    else {
        printf("Invalid flag.\n");
        return 1;
    }

    return 0;
}