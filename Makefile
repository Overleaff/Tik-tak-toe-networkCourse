CC = gcc
CFLAGS = -Wall -g

lab3: 
	${CC} ${CFLAGS} client.c -o client
	${CC} ${CFLAGS} server.c -o server

clean: 
	rm -f server client 
	rm -r *dSYM
