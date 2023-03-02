#include <unistd.h>
#include <fcntl.h>

#define SIZE 1024

int main(int agrc, charmx agrv){
    char buffer[SIZE]; //alocação estática, não necessita free
    while((bytes_read = read(0, buffer, 100)) > 0){
        bytes_written = write(1, buffer, bytes_read);
    }
}
