//ao definir esta struct estamos a pedir espaco em memoria do tamanho 200+sizeof(age)
typedef struct Person{
    char name[200];
    int age;
} Person;

#define FILENAME "file_pessoas"

//API
int newPerson(char* name, int age);
int updateAge(char* name, int age);
int updateAge_v2(long pos, int age);
