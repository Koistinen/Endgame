all: endgame

endgame: main.c
	gcc -O2 -Wall -o endgame main.c
