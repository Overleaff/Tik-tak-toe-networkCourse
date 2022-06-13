compile:
	gcc -Wall -g3 -fsanitize=address -pthread server.c -o server
	gcc -Wall -g3 -fsanitize=address -pthread game.c -o game

clean: 
	rm -f server client game
	rm -r *dSYM
