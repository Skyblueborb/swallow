#include <stddef.h>
#include <stdlib.h>

#include "entity.h"
#include "physics.h"
#include "types.h"

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

static void update_star_color(Game* game, Star* star) {
    const int y = star->ent.y;
    const int height = game->main_win.rows;

    if (height == 0) {
        return;
    }

    const float star_progress = (float)y / (float)height;

    if (star_progress < 0.2F) {
        star->ent.color = C_YELLOW_5;
    } else if (star_progress < 0.4F) {
        star->ent.color = C_YELLOW_4;
    } else if (star_progress < 0.6F) {
        star->ent.color = C_YELLOW_3;
    } else if (star_progress < 0.8F) {
        star->ent.color = C_YELLOW_2;
    } else {
        star->ent.color = C_YELLOW_1;
    }
}

void move_stars(Game* game) {
    Star* current = game->entities.stars;
    Star* prev = NULL;
    const Swallow* swallow = game->entities.swallow;

    while (current != NULL) {
        update_star_color(game, current);

        const int s_top = current->ent.y;
        const int s_bot = current->ent.y + current->ent.height + current->ent.speed;

        const int t_top = swallow->ent.y;
        const int t_bot = swallow->ent.y + swallow->ent.height;

        if (s_bot >= t_top && s_top <= t_bot &&
            current->ent.x < swallow->ent.x + swallow->ent.width &&
            current->ent.x + current->ent.width > swallow->ent.x) {
            game->stars_collected++;
            current = remove_star(game, current, prev);
            continue;
        }

        const collision_t ret = process_entity_tick(game, &current->ent, STAR);

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

void spawn_star(Game* game) {
    Star* star = (Star*)malloc(sizeof(Star));
    if (!star) {
        return;
    }

    star->ent.width = 1;
    star->ent.height = 1;
    for (int i = 0; i < NUM_DIRECTIONS; i++) {
        star->ent.sprites[i] = "*";
    }

    star->ent.speed = (rand() % STAR_SPEED_MAX) + 1;

    int max_c = game->main_win.cols - star->ent.width - 2;
    if (max_c <= 0) {
        max_c = 1;
    }

    star->ent.x = 2 + (rand() % max_c);
    star->ent.y = 1;
    star->ent.color = C_YELLOW_5;
    for (int i = 0; i < NUM_DIRECTIONS; i++) {
        star->ent.anim_sprites[i] = NULL;
    }
    star->ent.anim_frame = 0;
    star->ent.anim_timer = 0;

    change_entity_direction(&star->ent, DIR_DOWN, star->ent.speed);

    star->next = game->entities.stars;
    game->entities.stars = star;
}

void free_stars(Game* game) {
    free_generic_list(game, game->entities.stars, offsetof(Star, next), offsetof(Star, ent));
    game->entities.stars = NULL;
}
