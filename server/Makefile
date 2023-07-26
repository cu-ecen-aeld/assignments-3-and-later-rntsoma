CC=$(CROSS_COMPILE)gcc

default: all

all: Server

Server: aesdsocket.c
	$(CC) -o aesdsocket aesdsocket.c

clean:
	rm aesdsocket
