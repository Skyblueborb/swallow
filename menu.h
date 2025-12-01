#ifndef MENU_H
#define MENU_H

#include "types.h"

void show_high_scores(Game* game, int row_start);
MenuOption show_start_menu(Game *game);
void get_username(Game *game);
char* select_level(Game* game);
void handle_menu_choice(Game* game, MenuOption choice);

#endif // MENU_H
