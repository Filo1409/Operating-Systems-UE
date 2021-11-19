CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -Wall -pedantic $(DEFS) -g
LDFLAGS = -lrt -pthread

OBJECTS = test1.o test2.o

all: test1 test2

test1: test1.o
	$(CC) -o $@ $^ $(LDFLAGS)

test2: test2.o 
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

test1.o: test1.c test.h

test2.o: test2.c test.h

clean:
	rm -rf *.o test1 test2