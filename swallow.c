#include <unistd.h>
#include "entity.h"
#include "graphics.h"
#include "hunter.h"
#include "physics.h"
#include "star.h"
#include "types.h"

#define ANIMATION_TICKS 5

#define ALBATROSS_TAXI_DURATION 20.0f
#define SAFE_ZONE_PADDING 10

static void handle_swallow_star(Game* game, Swallow* s) {
    Star* prev = NULL;
    const int tx = s->ent.x + s->ent.dx;
    const int ty = s->ent.y + s->ent.dy;

    Star* target = find_star_collision(game, &prev, tx, ty, s->ent.width, s->ent.height);

    if (target) {
        game->stars_collected++;
        remove_star(game, target, prev);
    }
}

static void handle_swallow_hunter(Game* game, Swallow* s) {
    Hunter* prev = NULL;
    const int tx = s->ent.x + s->ent.dx;
    const int ty = s->ent.y + s->ent.dy;

    Hunter* target = find_hunter_collision(game, &prev, tx, ty, s->ent.width, s->ent.height);

    s->hp -= target->damage;

    if (target) {
        remove_hunter(game, target, prev);
    }
}

static void update_swallow_color(Swallow* s) {
    if (s->hp >= 80) {
        s->ent.color = C_GREEN_5;
    } else if (s->hp >= 60) {
        s->ent.color = C_CYAN_3;
    } else if (s->hp >= 40) {
        s->ent.color = C_PURPLE_5;
    } else if (s->hp >= 20) {
        s->ent.color = C_RED_3;
    } else {
        s->ent.color = C_RED_5;
    }
}

void process_swallow(Game* game) {
    Swallow* s = game->entities.swallow;

    s->ent.anim_timer++;
    if (s->ent.anim_timer >= ANIMATION_TICKS) {
        s->ent.anim_timer = 0;
        s->ent.anim_frame = !s->ent.anim_frame;
    }

    const collision_t ret = process_entity_tick(game, &s->ent, SWALLOW);

    if (ret == STAR) {
        handle_swallow_star(game, s);
    } else if (ret == HUNTER) {
        handle_swallow_hunter(game, s);
    }
    update_swallow_color(s);
}

static int is_zone_safe(Game* game, int x, int y, int w, int h) {
    const int pad = 3;
    const int check_x = x - pad;
    const int check_y = y - pad;
    const int check_w = w + (pad * 2);
    const int check_h = h + (pad * 2);

    if (check_x < 2 || check_y < 2 || check_x + check_w >= game->main_win.cols - 2 ||
        check_y + check_h >= game->main_win.rows - 2) {
        return 0;
    }

    return check_occupancy_map(game->occupancy_map, game->main_win.rows, game->main_win.cols,
                               check_x, check_y, check_w, check_h) == EMPTY;
}

static void find_safe_zone(Game* game, int* safe_x, int* safe_y, int w, int h) {
    for (int i = 0; i < MAX_SAFE_ZONE_ATTEMPTS; i++) {
        const int tx = 2 + (rand() % (game->main_win.cols - SAFE_ZONE_PADDING));
        const int ty = 2 + (rand() % (game->main_win.rows - SAFE_ZONE_PADDING));
        if (is_zone_safe(game, tx, ty, w, h)) {
            *safe_x = tx;
            *safe_y = ty;
            return;
        }
    }
    *safe_x = -1;
}

static void init_taxi_sprite(entity_t* taxi, int x, int y) {
    taxi->x = x;
    taxi->y = y;
    taxi->width = 3;
    taxi->height = 3;
    taxi->direction = DIR_UP;
    taxi->color = C_PURPLE_5;
    taxi->sprites[DIR_UP] =
            "#^#"
            "#o#"
            "###";
}

static void draw_static_scene(Game* game) {
    draw_main(game);
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
}

static void run_taxi_animation(Game* game, entity_t* taxi, int target_x, int target_y) {
    float cur_x = taxi->x;
    float cur_y = taxi->y;
    const float dx = (target_x - cur_x) / ALBATROSS_TAXI_DURATION;
    const float dy = (target_y - cur_y) / ALBATROSS_TAXI_DURATION;

    for (int i = 0; i < 20; i++) {
        remove_sprite(game, taxi);

        draw_static_scene(game);

        cur_x += dx;
        cur_y += dy;
        taxi->x = (int)cur_x;
        taxi->y = (int)cur_y;

        draw_sprite(game, taxi);
        wnoutrefresh(game->main_win.window);
        doupdate();
        usleep(30000);
    }
    remove_sprite(game, taxi);
}

void init_swallow(Game* game, Swallow* swallow) {
    entity_t* s = &swallow->ent;

    swallow->hp = SWALLOW_HP;
    s->width = SWALLOW_SIZE;
    s->height = SWALLOW_SIZE;
    s->x = game->main_win.cols / 2;
    s->y = game->main_win.rows / 2;
    s->speed = SWALLOW_SPEED;
    s->direction = DIR_RIGHT;
    s->dx = s->speed;
    s->dy = 0;
    s->color = C_GREEN_5;
    s->sprites[DIR_UP] =
            " ^ "
            "/o\\"
            ".Y.";
    s->sprites[DIR_DOWN] =
            "_w_"
            "\\o/"
            " v ";
    s->sprites[DIR_LEFT] =
            " /."
            "<o="
            " \\.";
    s->sprites[DIR_RIGHT] =
            ".\\ "
            "=o>"
            "./ ";

    // Altenative sprite for wing flapping
    s->anim_frame = 0;
    s->anim_timer = 0;
    s->anim_sprites[DIR_UP] =
            " ^ "
            "^o^"
            ".Y.";
    s->anim_sprites[DIR_DOWN] =
            "_w_"
            "vov"
            " v ";
    s->anim_sprites[DIR_LEFT] =
            " --"
            "<o="
            " --";
    s->anim_sprites[DIR_RIGHT] =
            "-- "
            "=o>"
            "-- ";
}

void call_albatross_taxi(Game* game) {
    if (game->albatross_cooldown > 0) return;

    Swallow* s = game->entities.swallow;
    int safe_x, safe_y;

    find_safe_zone(game, &safe_x, &safe_y, s->ent.width, s->ent.height);
    if (safe_x == -1) return;

    remove_entity(game, &s->ent);

    entity_t taxi = {0};
    init_taxi_sprite(&taxi, s->ent.x, s->ent.y);

    run_taxi_animation(game, &taxi, safe_x, safe_y);

    s->ent.x = safe_x;
    s->ent.y = safe_y;
    update_occupancy_map(game->occupancy_map, game->main_win.rows, game->main_win.cols, &s->ent,
                         SWALLOW);

    game->albatross_cooldown = game->config.albatross_cooldown;
}
