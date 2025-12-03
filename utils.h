#ifndef UTILS_H
#define UTILS_H

#include "types.h"

void strip_newline(char* str);
void init_curses();

void setup_windows(WIN* main_win, WIN* status_win, const conf_t* config);
void setup_menu_window(WIN* menu_win);

void free_occupancy_map(Game* game);
void init_occupancy_map(Game* game);

void change_game_speed(Game* game, increment_t increment);

int load_levels(char*** files);

#endif  // UTILS_H
