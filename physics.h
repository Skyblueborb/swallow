#ifndef PHYSICS_H
#define PHYSICS_H

#include "types.h"

void update_occupancy_map(char **occupancy_map, int rows, int cols, entity_t *ent, char map_representation);
collision_t check_occupancy_map(char** occupancy_map, int rows, int cols, int x, int y, int width,int height);

void change_entity_direction(entity_t* entity, direction_t direction, int speed);
direction_t get_opposite_direction(direction_t direction);
void move_entity(entity_t* entity, int dx, int dy);
int attempt_move_entity(Game* game, entity_t* ent);
Hunter* find_hunter_collision(Game *game, Hunter **prev_out, int area_x, int area_y, int area_w, int area_h);
Star* find_star_collision(Game *game, Star **prev_out, int area_x, int area_y, int area_w, int area_h);
int is_touching(entity_t* s, entity_t* t);

#endif // PHYSICS_H
