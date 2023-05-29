#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h> 

// PIPE 1 -> Escrita || 0 -> Leitura

//Teste 2013/2014

// Exercicio 1

void time_handler(int signum) {
	kill(get_pid(),9);
}

int boundedrun(int argv, char *args[]) {

	if(signal(SIGAlARM,time_handler)==SIG_ERR) {
		perror("SIGALRM ERROR");
	}

	int status;
	int fd[2];
	if (pipe(fd) == -1) {
        perror("Pipe creation failed");
    }

	close(fd[1]);

	int time = args[1];
	int size = args[2];

	if(fork() == 0) {
		close(fd[0]);
		dup2(fd[1],1);
		close(fd[1]);

		char *devolve[argv-3];

		for(int i = 0; i < argv-3 ; i++) {
			devolve[i] = args[i+3];
		}

		devolve[argv-3] = NULL;

		_execvp(args[3],devolve);
		_exit(0)
	}
	wait(&status);
	alarm(time);

	char buffer [size];
	while((res=read(fd[0],buffer,1))!=0) {
		write(1,buffer,1);
	}
	close(fd[0]);
	return 0;
}

// Exercicio 2

int conta (int argv,char *args[]) {
	int status;
	int fd[2];
	if (pipe(fd) == -1) {
        perror("Pipe creation failed");
    }

    close(fd[1]);

    int count = 0;

    for(int i = 2; i < argv ; i++) {

    	char *buffer;

    	if(fork() == 0) {
    		close(fd[0]);
    		dup2(fd[1],1);
    		close(fd[1]);

    		_execlp("encontra","encontra",args[1],args[i],NULL);
    		_exit(0);
    	}

    	wait(&status);
    	while((res=read(fd[0],buffer,1))!=0);

    	for(int x = 0; buffer[x] ; x++) {
    		if(buffer[x] == '\n' ||buffer[x] == '\0') count++;
    	}

    }
    close(fd[0]);

    return count;
}


//Teste 2015/2016

// Exercicio 1

int flag = 0;

void time_handler(int signum){
	printf("OK\n");
	if(flag == 0) alarm(3);

}

int CTRL(int argv, char *args[]) {

	if (signal(SIGALRM,time_handler)==SIG_ERR){
		perror("SIGALRM ERROR");
	}

	flag = 0;
	alarm(3);

	int status;
	int fd[2];
	pipe(fd);
	close(fd[1]);

	for(int i = 1; i < argv ; i++) {

		time_t seconds;
		time(&seconds);
		int filho;

		if((filho = fork()) == 0) {
			close(fd[0]);
			dup2(fd[1],1);
			close(fd[1]);

			_execlp(args[i],args[i],NULL);
			_exit(0);
		}

		wait(&status);
		flag = 1;
		printf("Tempo de processamento do filho %d = %ld\n", seconds);
	}

	close(fd[0]);
	return 0;
}

// Exercicio 2

char *creat_captcha_file(char *palavra) {

	// Criaçaõ do pipe cliente
	char *strPidd = malloc(128);
    sprintf(strPidd, "/tmp/%d", getpid());

    int cliente = mkfifo(strPidd, 0644);
    if(cliente < 0){
        perror("Pipe cliente nao criado!\n");
        _exit(0);
    }

    // Abertura do pipe cliente
    if((cliente= open(strPidd, O_WRONLY))< 0){
        perror("Cliente não inicializado\n");
        exit(1);
    }

    if((server= open("/tmp/server", O_RDONLY))< 0){
        perror("Pipe server encerrado!\n");
        _exit(0);
    } else {
        printf("Conecção establecida com servidor\n");
    }

    write(server,palavra,1024);

	char *ret = (char*)malloc(1024*sizeof(char));

    while(read(statOut, ret, 16384));

    close(cliente);
    unlink(pid);

	return ret;
}

int servidor(char *args[], int argv) {

	int server = mkfifo("/tmp/server", 0644);
    if(server < 0){
        perror("Pipe server nao criado!\n");
        _exit(0);
    }

    if((server= open("/tmp/server", O_RDONLY))< 0){
        perror("Pipe server encerrado!\n");
        _exit(0);
    }


    char *recebe[6];

    for(int i = 0; i < 6; i++) {
    	read(server,recebe[i],1);
    }

    char *buffer = (char*)malloc(1024*sizeof(char));

    size_t tamanho;

    tamanho = captcha(recebe,buffer);

    if((server= open("/tmp/server", O_WRONLY))< 0){
        perror("Servidor não inicializado\n");
        exit(1);
    }
    else{
        printf("Conecção establecida com servidor\n");
    }

    write(cliente,buffer,16384);

    close(server);
    unlink("/tmp/server");
	return 0;
}


//Teste 2016/2017

// Exercicio 1

int paginas (char *args[]) {

	int status;
	int count = 0;

	for(int i = 0; args[i] != NULL; i++) count++;

	int pipes[count][2];

	for(int i = 0; i < count; i++) {
		pipe(pipes[i]);
		fcntl(pipes[i][0], F_SETFL, O_NONBLOCK); // para de ler quando não tem mais nada
		close(pipes[i][1]);
	}



	for(int i = 0; i < count; i++) {

		if(fork() == 0) {
			close(pipes[i][0]);
			dup2(pipes[i][1],1);
			close(pipes[i][1]);

			_execlp(args[i],args[i],NULL);
			_exit(0);
		}
		wait(&status);
	}

	int flag = count;
	while(flag != 0) {
		for(int i = 0; i < count; i++) {
				for(int x = 0; x < 10; x++) {
					char buffer[1] = {' '};
					while(buffer[0] != '\n') {
					if(read(pipes[i][0],buffer[0],1) < 1) {
						flag--;
						waitss[i] = 1;
					}
					write(1,buffer,1);
				}
			}
		}
	}

	for(int i = 0; i < count; i++) {
		pipe(pipes[i]);
		close(pipes[i][0]);
	}


	return 0;
}

// Teste 2020/2021

// Exercicio 1

int servidor () {

	int server = mkfifo("/tmp/server",0644);
	if(server < 0){
        perror("Pipe server nao criado!\n");
        _exit(0);
    }

    open(server,O_WRONLY);

    int fildes[9];

    for(int i = 0; i < 9; i++) {
    	char nome_ficheiro[7];
        sprintf(nome_ficheiro, "zona_%d", i);
        fildes[i] = open(nome_ficheiro, O_WRONLY | O_CREAT, 0644);
    }

    char buffer[1024];
    while(read(server,buffer,1024) != 0) {
    	char *save_buf = buffer;
    	char **fields;
    	while ((fields = parse_entry(&save_buf))) {
    		char line
                [strlen(fields[0]) + strlen(fields[1]) + strlen(fields[2]) + 3];
            int to_write =
                sprintf(line, "%s %s %s\n", fields[0], fields[1], fields[2]);
            write(fildes[atoi(fields[2])], line, to_write);
    	} 
    }

    for(int i = 0; i < 9; i++) close(fildes[i]);

    close(server);
    unlink("/tmp/server");
	return 0;
}

// Exercicio 2

int vacinados (char* regiao, int idade) {
	int fd[2];
	int fd1[2];
	pipe(fd);
	pipe(fd1[2]);

	char idade_string[6];
    sprintf(idade_string, " %d ", idade);

	if(fork() == 0) {
		close(fd[0]);
		dup2(fd[1],1);
		close(fd[1]);

		_execlp("grep","grep",idade_string,regiao,NULL);
		_exit(0);
	}

	close(fd[1]);

	if (fork() == 0) {
        close(fd1[0]);
        dup2(fd[0], 0);
        close(fd[0]);
        dup2(fd1[1], 1);
        close(fd1[1]);
        execlp("wc", "wc", "-l", NULL);
    }

    close(fd[0]);
    close(fd1[1]);


    int read_bytes = 0;
    char buf[1025];
    read_bytes = read(pipes2[0], buf, 1024);

    close(fd1[0]);
    buf[read_bytes] = 0;
    return atoi(buf);
}

// Exercicio 3

bool vacinado(char* cidadao) {
    int pids[9];
    for (int i = 0; i < 9; i++) {
        if ((pids[i] = fork()) == 0) {
            char ficheiro[9];
            sprintf(ficheiro, "regiao_%d", i);
            execlp("grep", "grep", cidadao, ficheiro, NULL);
        }
    }
    int status;
    bool b = false;
    while (wait(&status) != -1) {
        if (WIFEXITED(status)) {
            if (WEXITSTATUS(status) == 0) {
                b = true;
                for (int i = 0; i < 9; i++) {
                    kill(pids[i], SIGKILL);
                }
            }
        }
    }
    return b;
}


















