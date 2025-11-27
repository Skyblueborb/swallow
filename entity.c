#include <stddef.h>

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
    hun->ent.color = t->color;
    hun->bounces = t->bounces;
    hun->damage = t->damage;
    hun->next = NULL;

    return hun;
}

static void setup_hunter_physics(Hunter* hun, int x, int y, direction_t dir) {
    hun->ent.x = x;
    hun->ent.y = y;
    change_entity_direction(&hun->ent, dir, hun->ent.speed);
}

static int is_touching(entity_t* s, entity_t* t) {
    int tolerance = 1;

    if (s->x < t->x + t->width + tolerance && s->x + s->width > t->x - tolerance &&
        s->y < t->y + t->height + tolerance && s->y + s->height > t->y - tolerance) {
        return 1;
    }
    return 0;
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

void spawn_star(Game* game) {
    Star* star = malloc(sizeof(Star));
    if (!star) return;

    star->ent.width = 1;
    star->ent.height = 1;
    for (int i = 0; i < NUM_DIRECTIONS; i++) star->ent.sprites[i] = "*";

    star->ent.speed = (rand() % 3) + 1;

    int max_c = game->main_win.cols - star->ent.width - 2;
    if (max_c <= 0) max_c = 1;

    star->ent.x = 2 + (rand() % max_c);
    star->ent.y = 1;
    star->ent.color = PAIR_STAR;

    change_entity_direction(&star->ent, DIR_DOWN, star->ent.speed);

    star->next = game->entities.stars;
    game->entities.stars = star;
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

static void* remove_generic_node(Game* game, void** head_ref, void* current, void* prev,
                                 size_t next_offset, size_t ent_offset) {
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

Hunter* remove_hunter(Game* game, Hunter* current, Hunter* prev) {
    return (Hunter*)remove_generic_node(game, (void**)&game->entities.hunters, current, prev,
                                        offsetof(Hunter, next), offsetof(Hunter, ent));
}

Star* remove_star(Game* game, Star* current, Star* prev) {
    return (Star*)remove_generic_node(game, (void**)&game->entities.stars, current, prev,
                                      offsetof(Star, next), offsetof(Star, ent));
}

void collect_stars(Game* game) {
    Star* current = game->entities.stars;
    Star* prev = NULL;
    Swallow* swallow = game->entities.swallow;

    while (current != NULL) {
        if (is_touching(&current->ent, &swallow->ent)) {
            game->stars_collected++;
            current = remove_star(game, current, prev);
        } else {
            prev = current;
            current = current->next;
        }
    }
}

void move_stars(Game* game) {
    Star* current = game->entities.stars;
    Star* prev = NULL;
    Swallow* swallow = game->entities.swallow;

    while (current != NULL) {
        int s_top = current->ent.y;
        int s_bot = current->ent.y + current->ent.height + current->ent.speed;

        int t_top = swallow->ent.y;
        int t_bot = swallow->ent.y + swallow->ent.height;

        if (s_bot >= t_top && s_top <= t_bot &&
            current->ent.x < swallow->ent.x + swallow->ent.width &&
            current->ent.x + current->ent.width > swallow->ent.x) {
            game->stars_collected++;
            current = remove_star(game, current, prev);
            continue;
        }

        collision_t ret = process_entity_tick(game, &current->ent, STAR);

        if (ret != EMPTY) {
            if (ret == SWALLOW) {
                game->stars_collected++;
            }
            current = remove_star(game, current, prev);
        } else {
            prev = current;
            current = current->next;
        }
    }
}

static int resolve_hunter_collision(Game* game, Hunter** curr, Hunter* prev, collision_t ret) {
    Hunter* h = *curr;
    if (ret != EMPTY) {
        change_entity_direction(&h->ent, get_opposite_direction(h->ent.direction), h->ent.speed);
        h->bounces--;

        if (ret == HUNTER || ret == SWALLOW) {
            if (ret == SWALLOW) game->entities.swallow->hp -= h->damage;
            *curr = remove_hunter(game, h, prev);
            return 1;
        }
    }

    if (h->bounces <= 0) {
        *curr = remove_hunter(game, h, prev);
        return 1;
    }
    return 0;
}

void process_hunters(Game* game) {
    Hunter* current = game->entities.hunters;
    Hunter* prev = NULL;

    while (current != NULL) {
        collision_t ret = process_entity_tick(game, &current->ent, HUNTER);

        if (resolve_hunter_collision(game, &current, prev, ret)) {
            continue;
        }

        prev = current;
        current = current->next;
    }
}

static void handle_swallow_star(Game* game, Swallow* s) {
    Star* prev = NULL;
    int tx = s->ent.x + s->ent.dx;
    int ty = s->ent.y + s->ent.dy;

    Star* target = find_star_collision(game, &prev, tx, ty, s->ent.width, s->ent.height);

    if (target) {
        game->stars_collected++;
        remove_star(game, target, prev);
    }
}

static void handle_swallow_hunter(Game* game, Swallow* s) {
    Hunter* prev = NULL;
    int tx = s->ent.x + s->ent.dx;
    int ty = s->ent.y + s->ent.dy;

    Hunter* target = find_hunter_collision(game, &prev, tx, ty, s->ent.width, s->ent.height);

    s->hp -= target->damage;

    if (target) {
        remove_hunter(game, target, prev);
    }
}

static void update_swallow_color(Swallow *s) {
    if (s->hp >= 80) {
        s->ent.color = C_GREEN_5;
    }
    else if (s->hp >= 60) {
        s->ent.color = C_CYAN_3;
    }
    else if (s->hp >= 40) {
        s->ent.color = C_PURPLE_5;
    }
    else if (s->hp >= 20) {
        s->ent.color = C_RED_3;
    }
    else {
        s->ent.color = C_RED_5;
    }
}

void process_swallow(Game* game) {
    Swallow* s = game->entities.swallow;
    collision_t ret = process_entity_tick(game, &s->ent, SWALLOW);

    if (ret == STAR) {
        handle_swallow_star(game, s);
    } else if (ret == HUNTER) {
        handle_swallow_hunter(game, s);
    }
    update_swallow_color(s);
}
