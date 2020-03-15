CFLAGS = gcc -Wall

all:
	make clear
	make static
	make shared

static:
	$(CFLAGS) -c blocks.c
	ar rcs blockslib.h blocks.o

shared:
	$(CFLAGS) -c -fPIC blocks.c
	$(CFLAGS) -shared -o blockslib.so blocks.o

clear:
	rm -f *.o *.a *.so