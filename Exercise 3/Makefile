# @author Filip Markovic
# @programname client

CC= -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g

all: client client.o server server.o

client: client.o
	gcc -o client client.o

client.o: client.c
	gcc $(CC) -c client.c

server: server.o
	gcc -o server server.o

server.o: server.c
	gcc $(CC) -c server.c 

clean: 
	rm -rf client.o client server.o server