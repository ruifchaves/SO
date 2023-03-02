#include <unistd.h>
#include <stdio.h>


//./a.out ls -l -a
int main(int argc, char** argv){

    pid_t pid;
    int status, return_exec, return_value;


    if(argc == 1){
        return -1;
    } else {
        for(int i = 1; i < argc; i++){
            if((pid = fork()) == 0) {
                //codigo processo filho
                printf("[pid-filho %d: started | %s %s]\n", getpid(), argv[1], argv[i]);
                return_exec = execlp(argv[1], argv[1], argv[i], NULL);
                printf("[pid-filho %d: after exec | %s %s]\n", getpid(), argv[1], argv[i]);

                perror("reached return");
                _exit(return_exec);
            }
        }
    }


    for (int i = 1; i < argc; i++){
        pid_t terminated_pid = wait(&status);

        if(WIFEXITED(status)) {
            printf("[pid-pai %d: ended with Exit Status %d]\n", terminated_pid, WEXITSTATUS(status));
        } else {
            printf("[pid-pai %d: filho did not end\n");
        }
    }
    return 0;
}



