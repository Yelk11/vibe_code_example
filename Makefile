CC = gcc
CFLAGS = -Wall -Wextra

all: game

game: game.c
	$(CC) $(CFLAGS) -o game game.c

clean:
	rm -f game

.PHONY: all clean 