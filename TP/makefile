CC = gcc
CFLAGS = -Wall

all: folders server client

server: bin/monitor

client: bin/tracer

folders:
	@mkdir -p src obj bin tmp

bin/monitor: obj/monitor.o
	$(CC) -g obj/monitor.o -o bin/monitor

obj/monitor.o: src/monitor.c
	$(CC) $(CFLAGS) -g -c src/monitor.c -o obj/monitor.o

bin/tracer: obj/tracer.o
	$(CC) -g obj/tracer.o -o bin/tracer

-o obj/tracer.o: src/tracer.c
	$(CC) $(CFLAGS) -g -c src/tracer.c -o obj/tracer.o

clean:
	rm -f obj/* tmp/* bin/{tracer,monitor}