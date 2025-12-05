#include <stddef.h>

#include "types.h"

void update_occupancy_map(char** occupancy_map, int rows, int cols, entity_t* ent,
                          char map_representation) {
    const int x = ent->x;
    const int y = ent->y;
    const int width = ent->width;
    const int height = ent->height;

    for (int i = y; i < y + height; i++) {
        for (int j = x; j < x + width; j++) {
            if (i >= 0 && i < rows && j >= 0 && j < cols) {
                if (occupancy_map[i][j] != WALL) {
                    occupancy_map[i][j] = map_representation;
                }
            }
        }
    }
}

collision_t check_occupancy_map(char** occupancy_map, int rows, int cols, int x, int y, int width,
                                int height) {
    if (x <= 0 || y <= 0 || x + width >= cols || y + height >= rows) {
        return WALL;
    }
    for (int i = y; i < y + height; i++) {
        for (int j = x; j < x + width; j++) {
            if (occupancy_map[i][j] != EMPTY) return (collision_t)occupancy_map[i][j];
        }
    }
    return EMPTY;
}

void change_entity_direction(entity_t* entity, direction_t direction, int speed) {
    entity->dx = 0;
    entity->dy = 0;
    entity->direction = direction;

    switch (direction) {
        case DIR_UP:
            entity->dy = -speed;
            break;
        case DIR_DOWN:
            entity->dy = speed;
            break;
        case DIR_RIGHT:
            entity->dx = speed;
            break;
        case DIR_LEFT:
            entity->dx = -speed;
            break;
        default:
            break;
    }
}

direction_t get_opposite_direction(direction_t direction) {
    switch (direction) {
        case DIR_UP:
            return DIR_DOWN;
        case DIR_DOWN:
            return DIR_UP;
        case DIR_LEFT:
            return DIR_RIGHT;
        case DIR_RIGHT:
            return DIR_LEFT;
        default:
            return NUM_DIRECTIONS;
    }
}

void move_entity(entity_t* entity, int dx, int dy) {
    entity->x += dx;
    entity->y += dy;
}

collision_t attempt_move_entity(Game* game, entity_t* ent) {
    const int check_x = ent->x + ent->dx;
    const int check_y = ent->y + ent->dy;

    const collision_t ret =
            check_occupancy_map(game->occupancy_map, game->main_win.rows, game->main_win.cols,
                                check_x, check_y, ent->width, ent->height);
    if (ret == EMPTY) {
        move_entity(ent, ent->dx, ent->dy);
        return EMPTY;
    }
    return ret;
}

static void* find_generic_collision(void* head, void** prev_out, size_t next_offset,
                                    size_t ent_offset, int area_x, int area_y, int area_w,
                                    int area_h) {
    void* current = head;
    void* last = NULL;

    while (current != NULL) {
        const entity_t* ent = (entity_t*)((char*)current + ent_offset);

        const int ent_left = ent->x;
        const int ent_right = ent->x + ent->width;
        const int ent_top = ent->y;
        const int ent_bottom = ent->y + ent->height;

        const int area_right = area_x + area_w;
        const int area_bottom = area_y + area_h;

        if (ent_left < area_right && ent_right > area_x && ent_top < area_bottom &&
            ent_bottom > area_y) {
            if (prev_out) *prev_out = last;
            return current;
        }

        last = current;
        void* const* const next_ptr = (void**)((char*)current + next_offset);
        current = *next_ptr;
    }
    return NULL;
}

Hunter* find_hunter_collision(Game* game, Hunter** prev_out, int area_x, int area_y, int area_w,
                              int area_h) {
    return (Hunter*)find_generic_collision(game->entities.hunters, (void**)prev_out,
                                           offsetof(Hunter, next), offsetof(Hunter, ent), area_x,
                                           area_y, area_w, area_h);
}

Star* find_star_collision(Game* game, Star** prev_out, int area_x, int area_y, int area_w,
                          int area_h) {
    return (Star*)find_generic_collision(game->entities.stars, (void**)prev_out,
                                         offsetof(Star, next), offsetof(Star, ent), area_x, area_y,
                                         area_w, area_h);
}

int is_touching(entity_t* s, entity_t* t) {
    if (s->x < t->x + t->width + PHYSICS_TOUCHING_TOLERANCE &&
        s->x + s->width > t->x - PHYSICS_TOUCHING_TOLERANCE &&
        s->y < t->y + t->height + PHYSICS_TOUCHING_TOLERANCE &&
        s->y + s->height > t->y - PHYSICS_TOUCHING_TOLERANCE) {
        return 1;
    }
    return 0;
}
