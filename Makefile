SRC = main.c
CC = clang

all: swallow

swallow: $(SRC)
	$(CC) -O0 -g -Wall -Werror -Wextra $(SRC) -o swallow -lncurses

clean:
	rm -f swallow

.phony: all clean

