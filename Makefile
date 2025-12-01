SRC = main.c utils.c conf.c graphics.c physics.c entity.c swallow.c hunter.c star.c ranking.c menu.c game.c
CC = clang

NCURSES_PREFIX = $(shell brew --prefix ncurses)

CFLAGS = -O0 -g -Wall -Werror -Wextra -I$(NCURSES_PREFIX)/include
LDFLAGS = -L$(NCURSES_PREFIX)/lib

LDLIBS = -lncursesw

all: swallow

swallow: $(SRC)
	python3 count_chars.py
	$(CC) $(CFLAGS) $(LDFLAGS) $(SRC) -o swallow $(LDLIBS)

clean:
	rm -f swallow

.phony: all clean

