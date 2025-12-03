#ifndef HUNTER_H
#define HUNTER_H

#include "types.h"

void process_hunters(Game* game);
void spawn_hunter(Game* game);
Hunter* remove_hunter(Game* game, Hunter* current, Hunter* prev);
void free_hunters(Game* game);

#endif  // HUNTER_H
