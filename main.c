#include <ctype.h>
#include <curses.h>
#include <locale.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "conf.h"
#include "entity.h"
#include "graphics.h"
#include "physics.h"
#include "types.h"
#include "utils.h"

#include "hunter.h"
#include "menu.h"
#include "star.h"
#include "swallow.h"

void handle_input(Game* game, entity_t* swallow) {
    int ch = tolower(getch());

    if (ch != ERR) {
        switch (ch) {
            case 'q':
                game->running = 0;
                break;
            case 'w':
                change_entity_direction(swallow, DIR_UP, swallow->speed);
                break;
            case 's':
                change_entity_direction(swallow, DIR_DOWN, swallow->speed);
                break;
            case 'a':
                change_entity_direction(swallow, DIR_LEFT, swallow->speed);
                break;
            case 'd':
                change_entity_direction(swallow, DIR_RIGHT, swallow->speed);
                break;
            case 'o':
                change_game_speed(game, DOWN);
                break;
            case 'p':
                change_game_speed(game, UP);
                break;
            case 'e':
                call_albatross_taxi(game);
                break;
        }
    }
}

static void handle_star_movement(Game* game) {
    if (game->star_move_tick == 4) {
        game->star_move_tick = -1;
        move_stars(game);
    }
    game->star_move_tick++;
}

static void handle_star_spawner(Game* game) {
    int star_spawn_threshold = game->config.star_spawn * 10;

    game->star_spawn_tick++;
    if (game->star_spawn_tick >= star_spawn_threshold) {
        game->star_spawn_tick = 0;
        spawn_star(game);
    }
}

static void handle_hunter_spawner(Game* game) {
    float base_hunter_threshold = game->config.hunter_spawn * 10;

    float elapsed = game->config.timer - game->time_left;
    float reduction_factor = 1.0f - ((elapsed / 5.0f) * 0.05f);

    if (reduction_factor < 0.2f) reduction_factor = 0.2f;

    float current_hunter_threshold = base_hunter_threshold * reduction_factor;
    if (current_hunter_threshold < 5) current_hunter_threshold = 5;

    game->hunter_spawn_tick++;
    if (game->hunter_spawn_tick >= current_hunter_threshold) {
        game->hunter_spawn_tick = 0;
        spawn_hunter(game);
    }
}

static void check_game_over(Game* game) {
    if (game->stars_collected >= game->config.star_quota) {
        game->running = 0;

        float score = 0;
        score += game->stars_collected * game->config.score_stars_weight;
        score += game->time_left * game->config.score_time_weight;
        score += game->entities.swallow->hp * game->config.score_life_weight;

        game->score = score * game->config.level_nr;
        game->result = WINNER;
    }

    if (game->time_left <= 0 || game->entities.swallow->hp <= 0) {
        float score = 0;
        score += game->stars_collected * game->config.score_stars_weight;
        score += game->time_left * game->config.score_time_weight;
        score += game->entities.swallow->hp * game->config.score_life_weight;
        game->score = score;

        game->time_left = 0;
        game->running = 0;
        game->result = LOSER;
    }
}

static void reset_game_state(Game* game) {
    game->hunter_spawn_tick = 0;
    game->star_spawn_tick = 0;
    game->star_move_tick = 0;
    game->star_flicker_tick = 0;
    game->stars_collected = 0;
    game->albatross_cooldown = 0;
    srand(game->config.seed);

    if (game->entities.hunters != NULL) free_hunters(game);
    if (game->entities.stars != NULL) free_stars(game);
}

void game_loop(Game* game) {
    if (fabs(game->time_left - game->config.timer) < 0.001) {
        reset_game_state(game);
    }
    unsigned int sleep_us = 66666 / game->game_speed;
    float delta_seconds = (float)sleep_us / 1000000.0f;

    Swallow* swallow = game->entities.swallow;
    handle_input(game, &swallow->ent);

    process_swallow(game);
    process_hunters(game);
    collect_stars(game);
    handle_star_movement(game);

    handle_hunter_spawner(game);
    handle_star_spawner(game);
    if (game->albatross_cooldown > 0) {
        game->albatross_cooldown -= delta_seconds;
        if (game->albatross_cooldown < 0) game->albatross_cooldown = 0;
    }
    game->time_left -= delta_seconds;
    check_game_over(game);

    draw_status(game);
    draw_main(game);
    doupdate();

    usleep(sleep_us);
}

int main() {
    setlocale(LC_ALL, "");
    Game game = {0};

    init_curses();

    setup_menu_window(&game.main_win);

    get_username(&game);

    game.menu_running = 1;
    while (game.menu_running) {
        setup_menu_window(&game.main_win);
        MenuOption choice = show_start_menu(&game);
        menu_loop(&game, choice);
    }

    if (game.entities.hunters) free_hunters(&game);
    if (game.entities.stars) free_stars(&game);
    if (game.entities.swallow) free(game.entities.swallow);

    delwin(game.main_win.window);
    delwin(game.status_win.window);
    endwin();

    if (game.username) free(game.username);
    free_config(&game.config);

    return 0;
}
