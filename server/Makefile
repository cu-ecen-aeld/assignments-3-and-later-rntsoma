CC=$(CROSS_COMPILE)gcc

default: all

all: Server

Server: aesdsocket.c
	$(CC) -o aesdsocket aesdsocket.c

clean:
	@if [ -e aesdsocket ]; then \
		rm aesdsocket; \
	else \
		echo "myfile.txt does not exist, nothing to clean."; \
	fi

