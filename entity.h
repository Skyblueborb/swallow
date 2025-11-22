#ifndef ENTITY_H
#define ENTITY_H

#include "types.h"
#include "graphics.h"

int is_vertical(direction_t dir);
void change_entity_direction(entity_t *entity, direction_t direction, int speed);
void move_entity(entity_t *entity, int dx, int dy);
direction_t get_opposite_direction(direction_t direction);
int attempt_move_entity(Game *game, entity_t *ent);
collision_t process_entity_tick(Game* game, entity_t* ent, char representation);
void spawn_hunter(Game *game);
void remove_entity(Game *game, entity_t *ent);
Hunter* remove_hunter(Game *game, Hunter *current, Hunter *prev);
void process_hunters(Game *game);
void process_swallow(Game *game);

#endif // ENTITY_H

