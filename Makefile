all: endgame

endgame: main.c
	gcc -g -O2 -Wall -o endgame main.c

clean:
	rm endgame
