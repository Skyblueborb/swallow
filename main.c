#include <ctype.h>
#include <curses.h>
#include <locale.h>
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
#include "ranking.h"
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
    static int update_stars = 0;

    if (update_stars == 4) {
        update_stars = -1;
        move_stars(game);
    }
    update_stars++;
}

static void handle_star_spawner(Game* game) {
    static int star_spawn_counter = 0;
    int star_spawn_threshold = game->config.star_spawn * 10;

    star_spawn_counter++;
    if (star_spawn_counter >= star_spawn_threshold) {
        star_spawn_counter = 0;
        spawn_star(game);
    }
}

static void handle_hunter_spawner(Game* game) {
    static int hunter_spawn_counter = 0;
    float base_hunter_threshold = game->config.hunter_spawn * 10;

    float elapsed = game->config.timer - game->time_left;
    float reduction_factor = 1.0f - ((elapsed / 5.0f) * 0.05f);

    if (reduction_factor < 0.2f) reduction_factor = 0.2f;

    float current_hunter_threshold = base_hunter_threshold * reduction_factor;
    if (current_hunter_threshold < 5) current_hunter_threshold = 5;

    hunter_spawn_counter++;
    if (hunter_spawn_counter >= current_hunter_threshold) {
        hunter_spawn_counter = 0;
        spawn_hunter(game);
    }
}

static void check_game_over(Game* game) {
    if (game->entities.swallow->hp <= 0) {
        game->running = 0;
    }

    if (game->stars_collected >= game->config.star_quota) {
        game->running = 0;
        game->score += game->time_left * game->config.score_time_weight +
                       game->entities.swallow->hp * game->config.score_life_weight;
        game->score *= game->config.level_nr;
    }

    if (game->time_left <= 0) {
        game->time_left = 0;
        game->running = 0;
    }
}
void game_loop(Game* game) {
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
        game->albatross_cooldown--;
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
    srand(time(NULL));
    Game game = {0};

    init_curses();

    conf_t initial_conf = {0};
    initial_conf.window_height = 32;
    initial_conf.window_width = 100;

    setup_windows(&game.main_win, &game.status_win, &initial_conf);
    wclear(game.status_win.window);
    wrefresh(game.status_win.window);

    get_username(&game);

    game.menu_running = 1;
    while (game.menu_running) {
        MenuOption choice = show_start_menu(&game);
        menu_loop(&game, choice);
    }

    delwin(game.main_win.window);
    delwin(game.status_win.window);
    endwin();

    if (game.username) free(game.username);
    if (game.entities.swallow) free(game.entities.swallow);
    free_config(&game.config);
    free_occupancy_map(&game);
    if (game.entities.hunters != NULL) free_hunters(&game);
    if (game.entities.stars != NULL) free_stars(&game);

    return 0;
}
