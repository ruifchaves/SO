#include <sys/types.h>
#include <unistd.h>         //definições e declarações de chamadas
#include <fcntl.h>          //definição modos de abertura de ficheiro

//int open(const char *path, int oflags [, mode])              O_RDONLY, O_WRONLY, O_CREAT, O_*; 0640 equivale a rw-r-----
//ssize_t read(int fildes, void *buf, size_t nbyte)            apontador para zona de memórias de um tipo unspecified; número max de bytes a ler
//ssize_t write(int fildes,const void *buf, size_t nbyte)      retorna numero de bytes escritos ou erro. erro pode ser no open, permissoes, etc\ 
//off_t lseek(int fd, off_t offset, int whence)                 
//int close(int fildes)

#define BUFFER_SIZE = 50

int mycp(char* src, char* dst){
    int src_fd = open(src, O_RDONLY);
    if(src_fd < 0){                             //porque não tratamos = 2?
        perror("Error opening source file")
        return 1;
    }

    int dts_fd = open(dst, O_CREAT | O_TRUNC | O_WRONLY, 0660)
    //0660 can be read and written by the owner and the group that the file belongs to, but not by anyone else. 
    //The file cannot be executed as the 'execute' permission is not granted to anyone.
    if(dts_fd < 0) {
        perror("Error opening destination file");
        return 1;
    }

    char buffer[BUFFER_SIZE]; //create buffer
    ssize_t src_content;      //read bytes

    while ((src_content = read(src_fd, buffer, BUFFER_SIZE)) > 0)
        write(dts_fd, buffer, BUFFER_SIZE);

    close(src_fd);
    close(dts_fd);
    return 0;
}


int main(int args, char* argv[]) {
    return mycp(argv[1], argv[2])
}