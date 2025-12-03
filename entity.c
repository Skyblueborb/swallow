#include <stddef.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "entity.h"
#include "graphics.h"
#include "physics.h"
#include "types.h"

#define HUNTER_LOOKAHEAD_TICKS 10

void aim_at_target(entity_t* source, const entity_t* target) {
    int dx = target->x - source->x;
    int dy = target->y - source->y;

    if (dx > 0)
        source->dx = source->speed;
    else if (dx < 0)
        source->dx = -source->speed;
    else
        source->dx = 0;

    if (dy > 0)
        source->dy = source->speed;
    else if (dy < 0)
        source->dy = -source->speed;
    else
        source->dy = 0;

    if (abs(dx) > abs(dy)) {
        source->direction = (dx > 0) ? DIR_RIGHT : DIR_LEFT;
    } else {
        source->direction = (dy > 0) ? DIR_DOWN : DIR_UP;
    }
}

int check_intercept_course(const entity_t* h, const entity_t* s) {
    int future_h_x = h->x + (h->dx * HUNTER_LOOKAHEAD_TICKS);
    int future_h_y = h->y + (h->dy * HUNTER_LOOKAHEAD_TICKS);

    int current_dist = abs(s->x - h->x) + abs(s->y - h->y);
    int future_dist = abs(s->x - future_h_x) + abs(s->y - future_h_y);

    return (future_dist < current_dist);
}

collision_t process_entity_tick(Game* game, entity_t* ent, char representation) {
    remove_entity(game, ent);
    collision_t ret = attempt_move_entity(game, ent);
    update_occupancy_map(game->occupancy_map, game->main_win.rows, game->main_win.cols, ent,
                         representation);
    draw_sprite(game, ent);
    return ret;
}

void remove_entity(Game* game, entity_t* ent) {
    remove_sprite(game, ent);
    update_occupancy_map(game->occupancy_map, game->main_win.rows, game->main_win.cols, ent, EMPTY);
}

void* remove_generic_node(Game* game, void** head_ref, void* current, void* prev,
                          size_t next_offset, size_t ent_offset) {
    if (!current) return NULL;

    entity_t* ent = (entity_t*)((char*)current + ent_offset);
    remove_entity(game, ent);

    void** current_next_ptr = (void**)((char*)current + next_offset);
    void* next_node = *current_next_ptr;

    if (prev == NULL) {
        *head_ref = next_node;
    } else {
        void** prev_next_ptr = (void**)((char*)prev + next_offset);
        *prev_next_ptr = next_node;
    }

    free(current);

    return next_node;
}

void free_generic_list(Game* game, void* head, size_t next_offset, size_t ent_offset) {
    void* current = head;

    while (current != NULL) {
        entity_t* ent = (entity_t*)((char*)current + ent_offset);
        remove_entity(game, ent);

        void** next_ptr = (void**)((char*)current + next_offset);
        void* next_node = *next_ptr;

        free(current);

        current = next_node;
    }
}
