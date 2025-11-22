#ifndef UTILS_H
#define UTILS_H

#include "types.h"

void init_curses();
void setup_windows(WIN* main_win, WIN* status_win, const conf_t *config);
void free_occupancy_map(Game *game);
void init_occupancy_map(Game *game);
void change_game_speed(Game *game, increment_t increment);
void init_swallow(Game* game, Swallow* swallow);
int count_hunters(Game *game);
void free_hunters(Game *game);

#endif // UTILS_H
