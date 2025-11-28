#include <stddef.h>
#include <time.h>
#include <unistd.h>

#include "entity.h"
#include "graphics.h"
#include "physics.h"
#include "types.h"

void aim_at_target(entity_t* source, entity_t* target) {
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

int check_intercept_course(entity_t* h, entity_t* s) {
    // Look ahead 10 ticks
    int future_h_x = h->x + (h->dx * 10);
    int future_h_y = h->y + (h->dy * 10);

    int current_dist = abs(s->x - h->x) + abs(s->y - h->y);
    int future_dist = abs(s->x - future_h_x) + abs(s->y - future_h_y);

    return (future_dist < current_dist);
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

void* remove_generic_node(Game* game, void** head_ref, void* current, void* prev,
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

static int is_zone_safe(Game* game, int x, int y, int w, int h) {
    int pad = 3;
    int check_x = x - pad;
    int check_y = y - pad;
    int check_w = w + (pad * 2);
    int check_h = h + (pad * 2);

    if (check_x < 2 || check_y < 2 || check_x + check_w >= game->main_win.cols - 2 ||
        check_y + check_h >= game->main_win.rows - 2) {
        return 0;
    }

    collision_t ret = check_occupancy_map(game->occupancy_map, game->main_win.rows, game->main_win.cols, check_x, check_y, check_w, check_h);
    if (ret == EMPTY) return 1;
    return 0;
}

void call_albatross_taxi(Game* game) {
    if (game->albatross_cooldown > 0) return;

    int safe_x = -1, safe_y = -1;
    int w = game->entities.swallow->ent.width;
    int h = game->entities.swallow->ent.height;
    for (int i = 0; i < 50; i++) {
        int tx = 2 + (rand() % (game->main_win.cols - 10));
        int ty = 2 + (rand() % (game->main_win.rows - 10));
        if (is_zone_safe(game, tx, ty, w, h)) {
            safe_x = tx;
            safe_y = ty;
            break;
        }
    }
    if (safe_x == -1) return;

    Swallow* s = game->entities.swallow;
    float cur_x = s->ent.x;
    float cur_y = s->ent.y;
    float dx = (safe_x - cur_x) / 20.0f;
    float dy = (safe_y - cur_y) / 20.0f;

    remove_entity(game, &s->ent);

    entity_t taxi = {0};
    taxi.direction = DIR_UP;
    taxi.sprites[DIR_UP] =
            "#^#"
            "#o#"
            "###";
    taxi.color = C_PURPLE_5;
    taxi.width = 3;
    taxi.height = 3;

    for (int i = 0; i < 20; i++) {
        remove_sprite(game, &taxi);

        Hunter* hu = game->entities.hunters;
        while (hu) {
            draw_sprite(game, &hu->ent);
            hu = hu->next;
        }
        Star* st = game->entities.stars;
        while (st) {
            draw_sprite(game, &st->ent);
            st = st->next;
        }


        cur_x += dx;
        cur_y += dy;
        taxi.x = (int)cur_x;
        taxi.y = (int)cur_y;

        draw_sprite(game, &taxi);

        draw_main(game);

        wnoutrefresh(game->main_win.window);

        doupdate();
        usleep(30000);
    }
    remove_sprite(game, &taxi);

    s->ent.x = safe_x;
    s->ent.y = safe_y;

    game->albatross_cooldown = 300;
}
