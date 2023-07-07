CC=$(CROSS_COMPILE)gcc

default: all

all: Writer

Writer: writer.c
	$(CC) -o writer writer.c

clean:
	rm writer
