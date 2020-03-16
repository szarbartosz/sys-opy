CFLAGS = gcc -Wall

static:
	make clear
	$(CFLAGS) -c blocks.c
	ar crs libblocks.a blocks.o
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


optimO:
	$(CFLAGS) -c blocks.c
	ar crs libblocks.a blocks.o
	$(CFLAGS) -O main.c -o main -L. -lblocks

optimO2:
	$(CFLAGS) -c blocks.c
	ar crs libblocks.a blocks.o
	$(CFLAGS) -O2 main.c -o main -L. -lblocks

optimO3:
	$(CFLAGS) -c blocks.c
	ar crs libblocks.a blocks.o
	$(CFLAGS) -O3 main.c -o main -L. -lblocks

optimOs:
	$(CFLAGS) -c blocks.c
	ar crs libblocks.a blocks.o
	$(CFLAGS) -Os main.c -o main -L. -lblocks



tests: test1 test2 test3


test1:
	./main compare_files a.txt:b.txt remove_block 0 compare_files c.txt:d.txt remove_block 0


test2:
	./main compare_files a.txt:b.txt remove_operation 0 0 remove_operation 0 1 remove operation 0 2


test3:
	./main compare_files a.txt:b.txt c.txt:d.txt a.txt:e.txt b.txt:f.txt remove_block 0


clear:
	rm -f *.o *.a *.so main