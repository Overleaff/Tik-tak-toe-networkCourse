compile:
	gcc -Wall -g3 -fsanitize=address -pthread -lm server.c -o server
	gcc -Wall -g3 -fsanitize=address -pthread -lm game.c -o game

clean: 
	rm -f server client game tu
	rm -r *dSYM
	rm -r .vscode