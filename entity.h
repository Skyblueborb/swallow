#ifndef ENTITY_H
#define ENTITY_H

#include "types.h"

collision_t process_entity_tick(Game* game, entity_t* ent, char representation);
void remove_entity(Game *game, entity_t *ent);

void spawn_star(Game* game);
Star* remove_star(Game* game, Star* current, Star* prev);
void collect_stars(Game* game);
void move_stars(Game* game);

void aim_at_target(entity_t* source, entity_t* target);
int check_intercept_course(entity_t* h, entity_t* s);

void* remove_generic_node(Game* game, void** head_ref, void* current, void* prev,
                                 size_t next_offset, size_t ent_offset);
void free_generic_list(Game* game, void* head, size_t next_offset, size_t ent_offset);

#endif // ENTITY_H
