 CC = gcc -Wall 
 
compile:
	make clear
	$(CC) server.c -o server -lrt
	$(CC) client.c -o client -lrt

clear:
	rm -f server client