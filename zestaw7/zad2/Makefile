 CC = gcc -Wall 
 
compile:
	clear
	$(CC) receiver.c -o receiver -lrt -pthread
	$(CC) packer.c -o packer -lrt -pthread
	$(CC) sender.c -o sender -lrt -pthread
	$(CC) main.c -o main -lrt -pthread
clear:
	clear
	rm -f receiver packer sender main