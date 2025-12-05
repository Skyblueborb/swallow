#include <ctype.h>
#include <unistd.h>

#include "conf.h"
#include "graphics.h"
#include "hunter.h"
#include "menu.h"
#include "physics.h"
#include "ranking.h"
#include "star.h"
#include "swallow.h"
#include "types.h"
#include "utils.h"

#define GAME_OVER_INPUT_BLOCK 2000000

#define STAR_MOVE_TICKS 4

#define BASE_SPAWNER_MULTIPLIER 10

static void handle_game_input(Game* game, entity_t* swallow) {
    const int ch = tolower(getch());

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
    game->star_move_tick++;
    if (game->star_move_tick == STAR_MOVE_TICKS) {
        game->star_move_tick = 0;
        move_stars(game);
    }
}

static void handle_star_spawner(Game* game) {
    const int star_spawn_threshold = game->config.star_spawn * BASE_SPAWNER_MULTIPLIER;

    game->star_spawn_tick++;
    if (game->star_spawn_tick >= star_spawn_threshold) {
        game->star_spawn_tick = 0;
        spawn_star(game);
    }
}

/**
 * handle_hunter_spawner - Manages hunter generation with difficulty scaling
 * @game: Main game struct
 *
 * Calculates how often a hunter should be spawned based on hunter_spawn_escalation.
 * Subsequently it spawns the hunter if enough ticks has passed.
 *
 * RETURNS
 * Void.
 */
static void handle_hunter_spawner(Game* game) {
    const float base_hunter_threshold = game->config.hunter_spawn * 10;

    const float elapsed = game->config.timer - game->time_left;
    float reduction_factor =
            1.0f - ((elapsed / HUNTER_ESCALATION_FREQUENCY) * game->config.hunter_spawn_escalation);

    if (reduction_factor < 0.2f) reduction_factor = 0.2f;

    float current_hunter_threshold = base_hunter_threshold * reduction_factor;
    if (current_hunter_threshold < 5) current_hunter_threshold = 5;

    game->hunter_spawn_tick++;
    if (game->hunter_spawn_tick >= current_hunter_threshold) {
        game->hunter_spawn_tick = 0;
        spawn_hunter(game);
    }
}

/**
 * calculate_score - Computes the final game score
 * @game: Main game struct
 *
 * Formula: (Stars * Star_Weight + Time * Time_Weight + Life * Life_Weight) * Level_Nr * Result
 * Weights are defined in the configuration file.
 *
 * RETURNS
 * Calculated score presented as an integer.
 */
static int calculate_score(Game* game) {
    float score = 0;
    score += game->stars_collected * game->config.score_stars_weight;
    if (game->result == WINNER) {
        score += game->entities.swallow->hp * game->config.score_life_weight;
        score += game->time_left * game->config.score_time_weight;
        score *= game->config.level_nr * game->result;
    }

    return (int)score;
}

/**
 * check_game_over - Verifies win/loss conditions
 * @game: Main game struct
 *
 * Win:stars collected >= star Quota.
 * Loss: Time runs out or HP drops to 0.
 *
 * RETURNS
 * Void.
 */
static void check_game_over(Game* game) {
    if (game->stars_collected >= game->config.star_quota) {
        game->result = WINNER;
    } else if (game->time_left <= 0 || game->entities.swallow->hp <= 0) {
        game->result = LOSER;

        // Prevent negative values in status_win
        game->time_left = 0;
        game->entities.swallow->hp = 0;
    }

    if (game->result != UNKNOWN) game->running = 0;
    game->score = calculate_score(game);
}

static void reset_game_state(Game* game) {
    game->hunter_spawn_tick = 0;
    game->star_spawn_tick = 0;
    game->star_move_tick = 0;
    game->star_flicker_tick = 0;
    game->stars_collected = 0;
    game->albatross_cooldown = 0;
    game->result = UNKNOWN;
    srand(game->config.seed);

    if (game->entities.hunters != NULL) free_hunters(game);
    if (game->entities.stars != NULL) free_stars(game);
}

void game_loop(Game* game) {
    if (game->time_left == game->config.timer) {
        reset_game_state(game);
    }
    const unsigned int sleep_us = 66666 / game->game_speed;
    const float delta_seconds = (float)sleep_us / 1000000.0f;

    Swallow* swallow = game->entities.swallow;
    handle_game_input(game, &swallow->ent);

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

void start_game(Game* game) {
    char* level_path = select_level(game);
    free_config(&game->config);
    game->config = read_config(level_path);
    free(level_path);

    delwin(game->main_win.window);
    delwin(game->status_win.window);
    setup_windows(&game->main_win, &game->status_win, &game->config);

    srand(game->config.seed);
    init_occupancy_map(game);

    game->running = 1;
    game->game_speed = game->config.min_speed;
    game->time_left = game->config.timer;
    game->score = 0.f;

    if (game->entities.swallow == NULL) {
        game->entities.swallow = (Swallow*)malloc(sizeof(Swallow));
        if (!game->entities.swallow) exit(1);
    }
    init_swallow(game, game->entities.swallow);

    while (game->running) {
        game_loop(game);
    }

    save_ranking(game);

    free_hunters(game);
    free_stars(game);
    free_occupancy_map(game);
}

void end_game(Game* game) {
    setup_menu_window(&game->main_win);
    nodelay(game->main_win.window, FALSE);
    draw_game_over(game, game->main_win.cols / 2, 1);
    show_high_scores(game, 10);
    flushinp();
    usleep(GAME_OVER_INPUT_BLOCK);
    wgetch(game->main_win.window);
    nodelay(game->main_win.window, TRUE);
}
