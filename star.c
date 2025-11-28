#include <stddef.h>

#include "entity.h"
#include "physics.h"
#include "types.h"

static void handle_collection(Game* game) {
    game->stars_collected++;
    game->score += game->config.score_stars_weight;
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
            handle_collection(game);
            current = remove_star(game, current, prev);
        } else {
            prev = current;
            current = current->next;
        }
    }
}

void move_stars(Game* game) {
    game->star_flicker_tick++;

    int cycle_step = (game->star_flicker_tick / 2) % 8;
    int color_offset = (cycle_step > 4) ? (8 - cycle_step) : cycle_step;

    ColorPair flicker_color = C_YELLOW_1 + color_offset;

    Star* current = game->entities.stars;
    Star* prev = NULL;
    Swallow* swallow = game->entities.swallow;

    while (current != NULL) {
        current->ent.color = flicker_color;

        int s_top = current->ent.y;
        int s_bot = current->ent.y + current->ent.height + current->ent.speed;

        int t_top = swallow->ent.y;
        int t_bot = swallow->ent.y + swallow->ent.height;

        if (s_bot >= t_top && s_top <= t_bot &&
            current->ent.x < swallow->ent.x + swallow->ent.width &&
            current->ent.x + current->ent.width > swallow->ent.x) {
            handle_collection(game);
            current = remove_star(game, current, prev);
            continue;
        }

        collision_t ret = process_entity_tick(game, &current->ent, STAR);

        if (ret != EMPTY) {
            if (ret == SWALLOW) {
                handle_collection(game);
            }
            current = remove_star(game, current, prev);
        } else {
            prev = current;
            current = current->next;
        }
    }
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

void free_stars(Game* game) {
    free_generic_list(game, game->entities.stars, offsetof(Star, next), offsetof(Star, ent));
    game->entities.stars = NULL;
}
