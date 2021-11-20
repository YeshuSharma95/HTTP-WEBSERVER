CC = gcc

all : server

server :
	$(CC) -o webserver webserver.c -pthread

clean :
	rm server

