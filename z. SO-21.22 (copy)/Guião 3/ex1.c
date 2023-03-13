#include <stdio.h>
#include <unistd.h>

int main (int argc, char** argv){
    int return_exec;
    char* exec_args[] = {"/bin/ls", "-l", NULL};

    printf("msg1\n");

    return_exec = execl("/bin/ls", "ls", "-l", NULL);
    //return_exec = execlp("ls", "ls", "-l", NULL);
    //return_exec = execv("/bin/ls", exec_args);
    //return_exec = execvp("ls", exec_args);
    
    printf("msg2\n");
    perror("Reached return");
    
    return 0;
}