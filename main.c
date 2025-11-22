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
        }
    }
}

void game_loop(Game* game) {
    static int hunter_spawn_counter = 0;
    int hunter_spawn_threshold = game->config.hunter_spawn * 10;
    static int star_spawn_counter = 0;
    static char update_stars = 0;
    int star_spawn_threshold = game->config.star_spawn * 10;
    Swallow* swallow = game->entities.swallow;
    handle_input(game, &swallow->ent);

    process_swallow(game);

    process_hunters(game);

    collect_stars(game);
    if (update_stars == 4) {
        update_stars = -1;
        move_stars(game);
    }
    update_stars++;

    hunter_spawn_counter++;
    if (hunter_spawn_counter == hunter_spawn_threshold) {
        hunter_spawn_counter = 0;
        spawn_hunter(game);
    }
    star_spawn_counter++;
    if (star_spawn_counter == star_spawn_threshold) {
        star_spawn_counter = 0;
        spawn_star(game);
    }

    if (swallow->hp <= 0) {
        exit(1);
    }

    if (game->stars_collected >= game->config.star_quota) {
        exit(1);
    }

    draw_status(game);
    draw_main(game);

    doupdate();

    usleep(66666 / game->game_speed);
}

int main() {
    setlocale(LC_ALL, "");
    srand(time(NULL));
    Game game = {0};
    game.config = read_config("config.txt");
    game.running = 1;
    game.game_speed = 3;

    init_curses();

    setup_windows(&game.main_win, &game.status_win, &game.config);
    init_occupancy_map(&game);

    Swallow swallow;
    init_swallow(&game, &swallow);

    game.entities.swallow = &swallow;

    while (game.running) {
        game_loop(&game);
    }

    delwin(game.main_win.window);
    delwin(game.status_win.window);
    endwin();
    free_config(&game.config);
    free_occupancy_map(&game);
    free_hunters(&game);
    return 0;
}
