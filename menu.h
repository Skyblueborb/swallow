#ifndef MENU_H
#define MENU_H

#include "types.h"

void show_high_scores(Game *game);
MenuOption show_start_menu(Game *game);
void menu_loop(Game* game, MenuOption choice);

#endif // MENU_H
