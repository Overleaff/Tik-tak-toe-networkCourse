all : client server
.PHONY : all
client : client.c
	gcc -pthread client.c -o client
server : server.c
	gcc -pthread server.c -o  server
clean :
	rm client server