#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

int open(const char *path, int oflags){

}

ssize_t read(int fildes, void *buf, size_t nbyte){

}

ssize_t write(int fildes,const void *buf, size_t nbyte){

}

off_t lseek(int fd, off_t offset, int whence){

}

int close(int fildes){
    
}