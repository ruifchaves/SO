#include <sys/types.h>
#include <unistd.h>         //chamadas ao sistema
#include <fcntl.h>          //modos de abertura de ficheiro

//int open(const char *path, int oflags [, mode])              O_RDONLY, O_WRONLY, O_CREAT, O_*; 0640 equivale a rw-r-----
//ssize_t read(int fildes, void *buf, size_t nbyte)            apontador para zona de memórias de um tipo unspecified; número max de bytes a ler
//ssize_t write(int fildes,const void *buf, size_t nbyte)      retorna numero de bytes escritos ou erro. erro pode ser no open, permissoes, etc\ 
//off_t lseek(int fd, off_t offset, int whence)                 
//int close(int fildes)

#define BUFFER_SIZE 1024

int mycat(){
    char buffer[buffer_size];
    size_t bytes_read;

    while((bytes_read = read(STDIN_FILENO, buffer, BUFFER_SIZE)) > 0)
        write(STDOUT_FILENO, buffer, bytes_read);

    return 0;
}

int main(int argc, char* argv[]){
    return mycat();
}