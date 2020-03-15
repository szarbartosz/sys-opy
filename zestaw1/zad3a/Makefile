CFLAGS = gcc -Wall

static:
	make clear
	$(CFLAGS) -c blocks.c
	ar rcs libblocks.a blocks.o
	$(CFLAGS) main.c -o main -L. -lblocks

shared:
	make clear
	$(CFLAGS) -fPIC -shared blocks.c -o libblocks.so
	$(CFLAGS) main.c -L. -Wl,-rpath=. -lblocks -o main


dynamic:
	make clear
	$(CFLAGS) -c -fPIC blocks.c
	$(CFLAGS) -shared -fPIC -o blockslib.so blocks.o
	$(CFLAGS) -o main dynamicMain.c -Wl,--no-as-needed -ldl -I.


clear:
	rm -f *.o *.a *.so main