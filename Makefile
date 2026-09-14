CC = gcc
CFLAGS = -Wall -g

all: client server

client: udpClient.c utils.c 
	$(CC) $(CFLAGS) udpClient.c utils.c -o client

server: udpServer.c utils.c 
	$(CC) $(CFLAGS) udpServer.c utils.c -o server

clean:
	rm -f client server