CFLAGS = gcc -Wall

compile:
	make clear
	$(CFLAGS) -c blocks.c
	ar rcs blockslib.a blocks.o
	$(CFLAGS) -c main.c
	$(CFLAGS) main.o blockslib.a -o main

clear:
	rm -f *.o *.a *.so