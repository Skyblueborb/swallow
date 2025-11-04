#include <stdio.h>
#include <curses.h>

int main(void) {

    initscr();
    printw("Hello Swallow");
    refresh();
    getch();
    endwin();

    return 0;
}
