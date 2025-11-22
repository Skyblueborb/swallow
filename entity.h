#ifndef ENTITY_H
#define ENTITY_H

#include "types.h"

collision_t process_entity_tick(Game* game, entity_t* ent, char representation);
void spawn_hunter(Game *game);
void remove_entity(Game *game, entity_t *ent);
Hunter* remove_hunter(Game *game, Hunter *current, Hunter *prev);
void process_hunters(Game *game);
void process_swallow(Game *game);

#endif // ENTITY_H
