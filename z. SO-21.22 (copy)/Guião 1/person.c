#include 'person.h'




int new_person(char* name, int age){
    Person p;
    strcpy(p.name, name);
    p.age = age;
    //append permite escrever logo no fim
    int fd = open(FILENAME, O_CREAT | O_APPEND | O_RDWR, 0600);
    int res = write(fd, &p, sizeof(Person));
    if(res<0){
        perror("Error creating person");
        return -1;
    }
    close(fd);
}
    
int person_change_age(char* name, int age){ //TODO
    Person p;

    int bytes_read;
    int res;

    
    while((bytes_read = read(fd, &p, sizeof(Pessoa))) < 0){

    }
    if()
    printf()

    if(strcmp(p.name, name)==0){ //igual
        p.age == age;
    }

    close(fd);
}






    strcpy(p.name, name);
    p.age = age;
    
    int fd = open(FILENAME, O_CREAT | O_APPEND | O_RDWR, 0600);
    while(read(fd, &p, sizeof(Pessoa)) < 0){
        lseek(fd, sizeof(Pessoa))
    }
        //recuar o tamanho de uma person para atualizar a idade, pq o offset ao comparar no read fica para o fim
        lseek(fd, -sizeof(Pessoa), SEEK_CUR); 
    strcmp(p.name, name)
    close(fd);
}




int person_change_age_v2(long pos, int age){

    int fd = open(FILENAME, O_CREAT | O_APPEND | O_RDWR, 0600);
    close(fd);
}


int main(int argc, char *argv[]){
    if(argc == 1 || argc == 2){
        return -1;
        break;
    }
    switch (argv[1]){
        case '-i':
            new_person(argv[2], argv[3]);
        case '-u':
            person_change_age(argv[2], argv[3]);
            //person_change_age_v2(argv[2], argv[2]);
    }
}

