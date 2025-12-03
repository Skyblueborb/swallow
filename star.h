#ifndef STAR_H
#define STAR_H

#include "types.h"

Star* remove_star(Game* game, Star* current, Star* prev);
void collect_stars(Game* game);
void move_stars(Game* game);
void spawn_star(Game* game);
void free_stars(Game* game);

#endif  // STAR_H
