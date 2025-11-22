#ifndef ENTITY_H
#define ENTITY_H

#include "types.h"

collision_t process_entity_tick(Game* game, entity_t* ent, char representation);
void remove_entity(Game *game, entity_t *ent);

void spawn_hunter(Game *game);
Hunter* remove_hunter(Game *game, Hunter *current, Hunter *prev);
void process_hunters(Game *game);

void spawn_star(Game* game);
Star* remove_star(Game* game, Star* current, Star* prev);
void collect_stars(Game* game);
void move_stars(Game* game);

void process_swallow(Game *game);

#endif // ENTITY_H
