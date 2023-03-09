#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdio.h>  //printf
#include <string.h> //stcmp
#include <stdlib.h> //atoi
#include <ctype.h>  //isDigit
#include <stdbool.h>//booleans
#include "person.h"


int addPerson(char* n, int a){

    //append permite escrever logo no fim
    int file_fd = open(FILENAME, O_CREAT | O_APPEND | O_WRONLY, 0660);

    if(file_fd < 0){
        perror("Error opening file...");
        return -1;
    }

    Person p; strcpy(p.name, n); p.age = a;

    //int offset = lseek(file_fd, 0, SEEK_END);
    int res = write(file_fd, &p, sizeof(Person));
    if(res < 0) {
        perror("Error adding new Person...");
        return 1;
    }
    
    int cur_pos = lseek(file_fd, 0, SEEK_CUR);
    int reg_number = cur_pos/sizeof(Person);

    close(file_fd);
    return reg_number;
}

int updateAge(char* n, int a){

    Person p;
    int bytes_read;
    int res;

    int file_fd = open(FILENAME, O_RDWR, 0600);
    if(file_fd < 0){
        perror("Error opening file; File does not exist...");
        return 1;
    }
    
    while((bytes_read = read(file_fd, &p, sizeof(Person)) > 0)){

        //debug
        printf("[READ] %s, %d\n", p.name, p.age);

        if(strcmp(p.name, n) == 0){
            p.age = a;

            res = lseek(file_fd, -sizeof(Person), SEEK_CUR);
            if(res < 0){
                perror("Error lseek...");
                return -1;
            }

            res = write(file_fd, &p, sizeof(Person));
            if(res < 0){
                perror("Error writing new age...");
                return -1;
            }

            //debug
            printf("[WROTE] %s, %d\n", p.name, p.age);

            close(file_fd);
            return 0;
        }
    }
    close(file_fd);
    return 1;
}



int updateAge_v2(long pos, int a){ //pos -> index da pessoa
    
    Person p;

    int file_fd = open(FILENAME, O_RDWR, 0600);
    if(file_fd < 0){
        perror("Error opening file; File does not exist...");
        return 1;
    }

    int seek_res = lseek(file_fd, (pos-1)*sizeof(Person), SEEK_SET);
    if(seek_res < 0){
        perror("Error lseek...");
        return 1;
    }

    int bytes_read = read(file_fd, &p, sizeof(Person));
    if(bytes_read < 0){
        perror("Error reading...");
        return 1;
    }

    //debug
    printf("[READ] %s, %d\n", p.name, p.age);

    seek_res = lseek(file_fd, -sizeof(Person), SEEK_CUR);
    if(seek_res < 0){
        perror("Error lseek...");
        return 1;
    }
    
    p.age = a;
    int res = write(file_fd, &p, sizeof(Person));
    if(res < 0){
        perror("Error writing new age...");
        return 1;
    }
    
    //debug
    printf("[WROTE] %s, %d\n", p.name, p.age);

    close(file_fd);
    return 0;
}

bool isNumber(char number[]){
    int i = 0;
    //checking for negative numbers
    if (number[0] == '-') i = 1;
    for (; number[i] != 0; i++){
        //if (number[i] > '9' || number[i] < '0')
        if (!isdigit(number[i])) return false;
    }
    return true;
}

int main (int argc, char* argv[]){
    if (argc != 4) {
        printf(
"\n---------------------------------\n\
Welcome to the program!\n\
Please select an option below:\n\
\n\
Flag  Function\n\
----  --------\n\
-i    Add a new person to the file\n\
-u    Update the age of a person in the file\n\
-o    Update the age of a person in the file (w/ register number)\n\
\n\
Usage: ./program [-i | -u] \"[name]\" [age]\n\
Usage: ./program [-x] [position] [age]\n\
---------------------------------\n\n");
        return 1;
    }

    char* flag = argv[1];
    char* name = argv[2];
    int   age  = atoi(argv[3]);

    char id[20] = "";
    if(strcmp(flag,"-i") == 0){
        int res = addPerson(name, age);
        snprintf(id, 20, "registo %d\n", res);
        write(STDOUT_FILENO, id, sizeof(id));
    } else if(strcmp(flag,"-u") == 0)
        updateAge(name, age);
    else if(strcmp(flag,"-o") == 0 && isNumber(argv[2]))
        updateAge_v2(atoi(argv[2]), age);
    else {
        printf("Invalid flag.");
        return 1;
    }

    return 0;
}