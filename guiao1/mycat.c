#include <sys/types.h>
#include <unistd.h>         //chamadas ao sistema
#include <fcntl.h>          //modos de abertura de ficheiro

//int open(const char *path, int oflags [, mode])              O_RDONLY, O_WRONLY, O_CREAT, O_*; 0640 equivale a rw-r-----
//ssize_t read(int fildes, void *buf, size_t nbyte)            apontador para zona de memórias de um tipo unspecified; número max de bytes a ler
//ssize_t write(int fildes,const void *buf, size_t nbyte)      retorna numero de bytes escritos ou erro. erro pode ser no open, permissoes, etc\ 
//off_t lseek(int fd, off_t offset, int whence)                 
//int close(int fildes)

#define BUFFER_SIZE 1024


//2. Implemente em C um programa mycat com funcionalidade similar ao comando cat, que lê do seu
//stdin e escreve para o seu stdout.
int mycat(){
    char buffer[buffer_size]; //alocação estática, não necessita free
    size_t bytes_read;
    size_t bytes_written;

    while((bytes_read = read(STDIN_FILENO, buffer, BUFFER_SIZE)) > 0) //STDIN_FILENO = 0
        bytes_written = write(STDOUT_FILENO, buffer, bytes_read);     //STDOUT_FILENO = 1

    return 0;
}

int main(int argc, char* argv[]){
    return mycat();
}