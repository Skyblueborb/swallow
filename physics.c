#include "types.h"

// We assume the fields are empty where we update
void update_occupancy_map(char** occupancy_map, int rows, int cols, entity_t* ent,
                          char map_representation) {
    int x = ent->x;
    int y = ent->y;
    int width = ent->width;
    int height = ent->height;

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

// Returns 0 for good positions, 1 for WALL, 2 for HUNTER, 3 for Swallow
collision_t check_occupancy_map(char** occupancy_map, int rows, int cols, int x, int y, int width,
                                int height) {
    if (x <= 0 || y <= 0 || x + width >= cols - 1 || y + height >= rows) {
        return WALL;  // Collision with Border Walls
    }
    for (int i = y; i < y + height; i++) {
        for (int j = x; j < x + width; j++) {
            if (occupancy_map[i][j] == WALL) {
                return WALL;
            }
            if (occupancy_map[i][j] == HUNTER) {
                return HUNTER;
            }
            if (occupancy_map[i][j] == SWALLOW) {
                return SWALLOW;
            }
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

int attempt_move_entity(Game* game, entity_t* ent) {
    int check_x = ent->x + ent->dx;
    int check_y = ent->y + ent->dy;

    collision_t ret =
            check_occupancy_map(game->occupancy_map, game->main_win.rows, game->main_win.cols,
                                check_x, check_y, ent->width, ent->height);
    if (ret == EMPTY) {
        move_entity(ent, ent->dx, ent->dy);
        return EMPTY;
    }
    return ret;
}

Hunter* find_hunter_collision(Game* game, Hunter** prev_out, int area_x, int area_y, int area_w,
                              int area_h) {
    Hunter* h = game->entities.hunters;
    Hunter* last = NULL;

    while (h != NULL) {
        int h_left = h->ent.x;
        int h_right = h->ent.x + h->ent.width;
        int h_top = h->ent.y;
        int h_bottom = h->ent.y + h->ent.height;

        int t_left = area_x;
        int t_right = area_x + area_w;
        int t_top = area_y;
        int t_bottom = area_y + area_h;

        if (h_left < t_right && h_right > t_left && h_top < t_bottom && h_bottom > t_top) {
            if (prev_out) *prev_out = last;
            return h;
        }

        last = h;
        h = h->next;
    }
    return NULL;
}
