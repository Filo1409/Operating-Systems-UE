CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -Wall -pedantic $(DEFS) -g
LDFLAGS = -lrt -pthread

OBJECTS = generator.o supervisor.o circbuffer.o

all: generator supervisor

generator: generator.o
	$(CC) -o $@ $^ $(LDFLAGS)

supervisor: supervisor.o 
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

generator.o: generator.c circbuffer.h

supervisor.o: supervisor.c circbuffer.h

clean:
	rm -rf *.o generator supervisor