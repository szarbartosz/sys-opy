 CC = gcc -Wall

compile:
	make clear
	$(CC) main.c -o main
	$(CC) consumer.c -o consumer
	$(CC) producer.c -o producer	

clear:
	clear
	rm -f main consumer producer pipe out.txt