 CC = gcc -Wall 
 
compile:
	clear
	$(CC) receiver.c -o receiver -lrt
	$(CC) packer.c -o packer -lrt
	$(CC) sender.c -o sender -lrt
	$(CC) main.c -o main -lrt
clear:
	clear
	rm -f receiver packer sender main