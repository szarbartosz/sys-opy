 CC = gcc -Wall 
 
compile:
	make clear
	$(CC) server.c -o server -lrt
	$(CC) client.c -o client -lrt

clear:
	make clear
	rm -f server client