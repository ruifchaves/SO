define WELCOME_BODY
############   BEM VINDO AO RASTREAMENTO E MONITORIZAÇÃO DA EXECUÇÃO DE PROGRAMAS!   ############

	TARGET
-Executar um programa individual:
    ./tracer execute -u "prog arg1 (...) argN"

-Executar uma pipeline de programas:
    ./tracer execute -p "progA arg1 (...) argN | progB arg1 (...) argN | progC arg1 (...) argN"

-Listar programas em execução:
    ./tracer status

-Total de tempo de execução de um conjunto de programas terminados:
    ./tracer stats-time PID1 PID2 (...) PIDN

-Número de vezes que um certo programa foi executado:
    ./tracer stats-command progA PID1 PID2 (...) PIDN

-Listar programas únicos executados:
    ./tracer stats-uniq PID1 PID2 (...) PIDN

	SERVIDOR
-Executar o Servidor:
    ./monitor PIDS-folder

#################################################################################################
endef

CC = gcc
CFLAGS = -Wall -Wno-unused-variable -Wno-unused-but-set-variable

all: folders server client

export WELCOME_BODY
welcome:
	@echo "$$WELCOME_BODY"

server: bin/monitor

client: bin/tracer

folders:
	@mkdir -p src obj bin tmp finished

bin/monitor: obj/monitor.o
	$(CC) -g obj/monitor.o -o bin/monitor

obj/monitor.o: src/common.h src/monitor.c 
	$(CC) $(CFLAGS) -g -c src/monitor.c -o obj/monitor.o

bin/tracer: obj/tracer.o
	$(CC) -g obj/tracer.o -o bin/tracer

-o obj/tracer.o: src/common.h src/tracer.c
	$(CC) $(CFLAGS) -g -c src/tracer.c -o obj/tracer.o

#não eliminar a pasta finished
clean:
	rm -f obj/* tmp/* bin/{tracer,monitor}