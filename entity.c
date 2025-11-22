#include <stdlib.h>
#include "graphics.h"
#include "physics.h"
#include "types.h"

void change_entity_direction(entity_t* entity, direction_t direction, int speed) {
    // 2. Update Direction and Velocity
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

void get_spawn_coordinates(Game *game, int w, int h, direction_t dir, int *x, int *y) {
    int max_r = game->main_win.rows - h - 2;
    int max_c = game->main_win.cols - w - 2;

    // Safety clamp to prevent modulo by zero/negative if window is tiny
    if (max_r <= 0) max_r = 1;
    if (max_c <= 0) max_c = 1;

    switch (dir) {
        case DIR_RIGHT: // Spawn on Left Edge, Random Y
            *x = 2;
            *y = 2 + (rand() % max_r);
            break;
        case DIR_LEFT:  // Spawn on Right Edge, Random Y
            *x = max_c;
            *y = 2 + (rand() % max_r);
            break;
        case DIR_DOWN:  // Spawn on Top Edge, Random X
            *x = 2 + (rand() % max_c);
            *y = 2;
            break;
        case DIR_UP:    // Spawn on Bottom Edge, Random X
            *x = 2 + (rand() % max_c);
            *y = max_r;
            break;
        default:
            break;
    }
}

Hunter* init_hunter_data(Game *game, int template_idx) {
    Hunter* hun = malloc(sizeof(Hunter));
    if (!hun) return NULL;

    HunterTypes *t = &game->config.hunter_templates[template_idx];

    for (int i = 0; i < NUM_DIRECTIONS; i++) {
        hun->ent.sprites[i] = t->sprites[i];
    }

    hun->ent.width = t->width;
    hun->ent.height = t->height;
    hun->ent.speed = t->speed;
    hun->bounces = t->bounces;
    hun->next = NULL;

    return hun;
}
void setup_hunter_physics(Hunter *hun, int x, int y, direction_t dir) {
    hun->ent.x = x;
    hun->ent.y = y;

    change_entity_direction(&hun->ent, dir, hun->ent.speed);
}

void spawn_hunter(Game *game) {
    int t_idx = rand() % 5;
    direction_t dir = (direction_t)(rand() % 4);

    int w = game->config.hunter_templates[t_idx].width;
    int h = game->config.hunter_templates[t_idx].height;

    int x, y;
    get_spawn_coordinates(game, w, h, dir, &x, &y);

    Hunter *new_hunter = init_hunter_data(game, t_idx);
    if (!new_hunter) return;

    setup_hunter_physics(new_hunter, x, y, dir);

    new_hunter->next = game->entities.hunters;
    game->entities.hunters = new_hunter;
}

void move_entity(entity_t* entity, int dx, int dy) {
    entity->x += dx;
    entity->y += dy;
}

int attempt_move_entity(Game* game, entity_t* ent) {
    int check_x = ent->x + ent->dx;
    int check_y = ent->y + ent->dy;

    collision_t ret = check_occupancy_map(game->occupancy_map, game->main_win.rows, game->main_win.cols, check_x, check_y, ent->width, ent->height);
    if(ret == EMPTY) {
        move_entity(ent, ent->dx, ent->dy);
        return EMPTY;
    }
    return ret;
}

// Actions that every entity should do every tick
collision_t process_entity_tick(Game* game, entity_t* ent, char representation) {
    remove_sprite(game, ent);
    update_occupancy_map(game->occupancy_map, game->main_win.rows, game->main_win.cols, ent, ' ');
    collision_t ret = attempt_move_entity(game, ent);
    update_occupancy_map(game->occupancy_map, game->main_win.rows, game->main_win.cols, ent,
                         representation);
    draw_sprite(game, ent);
    return ret;
}

void remove_entity(Game *game, entity_t *ent) {
    remove_sprite(game, ent);
    update_occupancy_map(game->occupancy_map, game->main_win.rows, game->main_win.cols, ent, ' ');
}

Hunter* remove_hunter(Game *game, Hunter *current, Hunter *prev) {
    remove_entity(game, &current->ent); // Clear from screen/map

    Hunter* to_free = current;
    Hunter* next_node = current->next;

    if (prev == NULL) {
        game->entities.hunters = next_node; // Head removal
    } else {
        prev->next = next_node; // Middle/Tail removal
    }

    free(to_free);
    return next_node; // Return the next valid link
}

// Returns the first hunter that overlaps with the specified rectangle area
Hunter* find_hunter_collision(Game *game, Hunter **prev_out, int area_x, int area_y, int area_w, int area_h) {
    Hunter *h = game->entities.hunters;
    Hunter *last = NULL;

    while (h != NULL) {
        // 1. Calculate Hunter Bounds
        int h_left   = h->ent.x;
        int h_right  = h->ent.x + h->ent.width;  // Exclusive
        int h_top    = h->ent.y;
        int h_bottom = h->ent.y + h->ent.height; // Exclusive

        // 2. Calculate Target Area Bounds (The Swallow)
        int t_left   = area_x;
        int t_right  = area_x + area_w;          // Exclusive
        int t_top    = area_y;
        int t_bottom = area_y + area_h;          // Exclusive

        // 3. AABB Collision Logic
        // Checks if the two rectangles overlap
        if (h_left < t_right && h_right > t_left &&
            h_top < t_bottom && h_bottom > t_top) {

            if (prev_out) *prev_out = last;
            return h;
        }

        last = h;
        h = h->next;
    }

    return NULL;
}

void process_hunters(Game *game) {
    Hunter *current = game->entities.hunters;
    Hunter *prev = NULL;
    while (current != NULL) {
        collision_t ret = process_entity_tick(game, &current->ent, HUNTER);

        if (ret != EMPTY) {
            change_entity_direction(&current->ent, get_opposite_direction(current->ent.direction), current->ent.speed);
            current->bounces--;
            switch(ret) {
                case HUNTER:
                    remove_hunter(game, current, prev);
                    break;
                case SWALLOW:
                    game->entities.swallow->hp -= 5;
                    remove_hunter(game, current, prev);
                    break;
                case WALL:
                default:
                    current->bounces--;
            }
        } else if (current->bounces <= 0) {
            current = remove_hunter(game, current, prev);
            continue;
        }
        prev = current;
        current = current->next;

    }
}

void process_swallow(Game *game) {
    Swallow *s = game->entities.swallow;
    collision_t ret = process_entity_tick(game, &s->ent, SWALLOW);

    if (ret != EMPTY) {
        switch(ret) {
            case HUNTER:
                s->hp -= 5;
                Hunter *prev = NULL;
                Hunter *to_free = find_hunter_collision(
                    game,
                    &prev,
                    s->ent.x + s->ent.dx, // Target X
                    s->ent.y + s->ent.dy, // Target Y
                    s->ent.width,         // Target Width
                    s->ent.height         // Target Height
                );

                if (to_free) {
                    remove_hunter(game, to_free, prev);
                }
                break;
            case SWALLOW:
            case WALL:
            default:
                break;
        }
    }
}
