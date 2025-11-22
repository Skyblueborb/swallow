#include "entity.h"
#include "graphics.h"
#include "physics.h"
#include "types.h"

static void get_spawn_coordinates(Game* game, int w, int h, direction_t dir, int* x, int* y) {
    int max_r = game->main_win.rows - h - 2;
    int max_c = game->main_win.cols - w - 2;

    if (max_r <= 0) max_r = 1;
    if (max_c <= 0) max_c = 1;

    switch (dir) {
        case DIR_RIGHT:
            *x = 2;
            *y = 2 + (rand() % max_r);
            break;
        case DIR_LEFT:
            *x = max_c;
            *y = 2 + (rand() % max_r);
            break;
        case DIR_DOWN:
            *x = 2 + (rand() % max_c);
            *y = 2;
            break;
        case DIR_UP:
            *x = 2 + (rand() % max_c);
            *y = max_r;
            break;
        default:
            break;
    }
}

static Hunter* init_hunter_data(Game* game, int template_idx) {
    Hunter* hun = malloc(sizeof(Hunter));
    if (!hun) return NULL;

    HunterTypes* t = &game->config.hunter_templates[template_idx];

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

static void setup_hunter_physics(Hunter* hun, int x, int y, direction_t dir) {
    hun->ent.x = x;
    hun->ent.y = y;
    change_entity_direction(&hun->ent, dir, hun->ent.speed);
}

void spawn_hunter(Game* game) {
    int t_idx = rand() % 5;
    direction_t dir = (direction_t)(rand() % 4);

    int w = game->config.hunter_templates[t_idx].width;
    int h = game->config.hunter_templates[t_idx].height;

    int x, y;
    get_spawn_coordinates(game, w, h, dir, &x, &y);

    Hunter* new_hunter = init_hunter_data(game, t_idx);
    if (!new_hunter) return;

    setup_hunter_physics(new_hunter, x, y, dir);

    new_hunter->next = game->entities.hunters;
    game->entities.hunters = new_hunter;
}

collision_t process_entity_tick(Game* game, entity_t* ent, char representation) {
    remove_sprite(game, ent);
    update_occupancy_map(game->occupancy_map, game->main_win.rows, game->main_win.cols, ent, ' ');
    collision_t ret = attempt_move_entity(game, ent);
    update_occupancy_map(game->occupancy_map, game->main_win.rows, game->main_win.cols, ent,
                         representation);
    draw_sprite(game, ent);
    return ret;
}

void remove_entity(Game* game, entity_t* ent) {
    remove_sprite(game, ent);
    update_occupancy_map(game->occupancy_map, game->main_win.rows, game->main_win.cols, ent, ' ');
}

Hunter* remove_hunter(Game* game, Hunter* current, Hunter* prev) {
    remove_entity(game, &current->ent);

    Hunter* to_free = current;
    Hunter* next_node = current->next;

    if (prev == NULL) {
        game->entities.hunters = next_node;
    } else {
        prev->next = next_node;
    }

    free(to_free);
    return next_node;
}

void process_hunters(Game* game) {
    Hunter* current = game->entities.hunters;
    Hunter* prev = NULL;
    while (current != NULL) {
        collision_t ret = process_entity_tick(game, &current->ent, HUNTER);

        if (ret != EMPTY) {
            change_entity_direction(&current->ent, get_opposite_direction(current->ent.direction),
                                    current->ent.speed);
            current->bounces--;
            switch (ret) {
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

void process_swallow(Game* game) {
    Swallow* s = game->entities.swallow;
    collision_t ret = process_entity_tick(game, &s->ent, SWALLOW);

    if (ret != EMPTY) {
        switch (ret) {
            case HUNTER:
                s->hp -= 5;
                Hunter* prev = NULL;
                Hunter* to_free =
                        find_hunter_collision(game, &prev, s->ent.x + s->ent.dx,
                                              s->ent.y + s->ent.dy, s->ent.width, s->ent.height);

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
