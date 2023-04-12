#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdio.h>  //printf
#include <string.h> //stcmp
#include <stdlib.h> //atoi
#include <ctype.h>  //isDigit
#include <stdbool.h>//booleans
#include <time.h>
#include "person.h"

#define FILENAME "file_pessoas"

//ao definir esta struct estamos a pedir espaco em memoria do tamanho 200+sizeof(age)
typedef struct Person {
    char name[200];
    int age;
} Person;

//API
int newPerson(char* name, int age);
int updateAge(char* name, int age);
int updateAge_v2(long pos, int age);
