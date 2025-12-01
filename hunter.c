#include <stddef.h>
#include <stdlib.h>

#include "entity.h"
#include "graphics.h"
#include "physics.h"
#include "types.h"

static void get_spawn_coordinates(Game* game, entity_t* hunter_ent, direction_t side) {
    int w = hunter_ent->width;
    int h = hunter_ent->height;
    int max_r = game->main_win.rows - h - 2;
    int max_c = game->main_win.cols - w - 2;
    if (max_r <= 0) max_r = 1;
    if (max_c <= 0) max_c = 1;

    int x = 2, y = 2;

    switch (side) {
        case DIR_RIGHT:
            x = 2;
            y = 2 + (rand() % max_r);
            break;
        case DIR_LEFT:
            x = max_c;
            y = 2 + (rand() % max_r);
            break;
        case DIR_DOWN:
            x = 2 + (rand() % max_c);
            y = 2;
            break;
        case DIR_UP:
            x = 2 + (rand() % max_c);
            y = max_r;
            break;
        default:
            break;
    }

    hunter_ent->x = x;
    hunter_ent->y = y;
}

static void setup_hunter_physics(Hunter* hun, int x, int y, direction_t dir) {
    hun->ent.x = x;
    hun->ent.y = y;
    change_entity_direction(&hun->ent, dir, hun->ent.speed);
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

    hun->state = HUNTER_IDLE;
    hun->state_timer = 0;
    hun->dash_cooldown = 0;
    hun->base_speed = t->speed;

    for (int i = 0; i < NUM_DIRECTIONS; i++) hun->ent.anim_sprites[i] = NULL;
    hun->ent.anim_frame = 0;
    hun->ent.anim_timer = 0;

    hun->next = NULL;

    return hun;
}

void spawn_hunter(Game* game) {
    int t_idx = rand() % game->config.hunter_templates_amount;
    direction_t dir = (direction_t)(rand() % NUM_DIRECTIONS);

    Hunter* new_hunter = init_hunter_data(game, t_idx);
    if (!new_hunter) return;

    float elapsed = game->config.timer - game->time_left;
    int bonus_bounces;
    if(game->config.hunter_bounce_escalation > 0)
        bonus_bounces = (int)(elapsed / game->config.hunter_bounce_escalation);
    new_hunter->bounces += bonus_bounces;

    get_spawn_coordinates(game, &new_hunter->ent, dir);

    setup_hunter_physics(new_hunter, new_hunter->ent.x, new_hunter->ent.y, dir);

    new_hunter->next = game->entities.hunters;
    game->entities.hunters = new_hunter;
}

Hunter* remove_hunter(Game* game, Hunter* current, Hunter* prev) {
    return (Hunter*)remove_generic_node(game, (void**)&game->entities.hunters, current, prev,
                                        offsetof(Hunter, next), offsetof(Hunter, ent));
}

static int resolve_hunter_collision(Game* game, Hunter** curr, Hunter* prev, collision_t ret) {
    Hunter* h = *curr;
    if (ret != EMPTY) {
        h->state = HUNTER_IDLE;
        h->ent.speed = h->base_speed;

        int hit_x = 0;
        int hit_y = 0;

        if (check_occupancy_map(game->occupancy_map, game->main_win.rows, game->main_win.cols,
                                h->ent.x + h->ent.dx, h->ent.y, h->ent.width,
                                h->ent.height) != EMPTY) {
            hit_x = 1;
        }

        if (check_occupancy_map(game->occupancy_map, game->main_win.rows, game->main_win.cols,
                                h->ent.x, h->ent.y + h->ent.dy, h->ent.width,
                                h->ent.height) != EMPTY) {
            hit_y = 1;
        }

        if (!hit_x && !hit_y) {
            hit_x = 1;
            hit_y = 1;
        }

        if (hit_x) h->ent.dx = -h->ent.dx;
        if (hit_y) h->ent.dy = -h->ent.dy;

        if (abs(h->ent.dx) > abs(h->ent.dy))
            h->ent.direction = (h->ent.dx > 0) ? DIR_RIGHT : DIR_LEFT;
        else
            h->ent.direction = (h->ent.dy > 0) ? DIR_DOWN : DIR_UP;

        h->bounces--;

        if (ret == HUNTER || ret == SWALLOW) {
            if (ret == SWALLOW) {
                game->entities.swallow->hp -= h->damage;
            }
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

static int handle_hunter_logic(Hunter* h, Swallow* s) {
    if (h->dash_cooldown > 0) {
        h->dash_cooldown--;
    }

    switch (h->state) {
        case HUNTER_IDLE:
            if (h->dash_cooldown <= 0 && !check_intercept_course(&h->ent, &s->ent)) {
                h->state = HUNTER_PAUSED;
                h->state_timer = HUNTER_IDLE_TICKS;
            }
            break;

        case HUNTER_PAUSED:
            h->state_timer--;
            if (h->state_timer <= 0) {
                h->state = HUNTER_DASHING;
                h->ent.speed = (h->base_speed * 2);
                if (h->ent.speed > 3) h->ent.speed = 2;

                aim_at_target(&h->ent, &s->ent);

                h->dash_cooldown = HUNTER_DASH_COOLDOWN_TICKS;
            }
            return 1;

        case HUNTER_DASHING:
            break;
    }
    return 0;
}

void process_hunters(Game* game) {
    Hunter* current = game->entities.hunters;
    Hunter* prev = NULL;

    while (current != NULL) {
        if (handle_hunter_logic(current, game->entities.swallow)) {
            draw_sprite(game, &current->ent);

            prev = current;
            current = current->next;
            continue;
        }
        collision_t ret = process_entity_tick(game, &current->ent, HUNTER);

        if (resolve_hunter_collision(game, &current, prev, ret)) {
            continue;
        }

        prev = current;
        current = current->next;
    }
}

void free_hunters(Game* game) {
    free_generic_list(game, game->entities.hunters, offsetof(Hunter, next), offsetof(Hunter, ent));
    game->entities.hunters = NULL;
}
