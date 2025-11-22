#include <stddef.h>
#include <stdio.h>
#include "types.h"

void draw_sprite(Game* game, entity_t* entity) {
    char* sprite_grid = entity->sprites[entity->direction];
    WINDOW* win = game->main_win.window;

    for (int sprite_y = 0; sprite_y < entity->height; sprite_y++) {
        for (int sprite_x = 0; sprite_x < entity->width; sprite_x++) {
            int screen_y = entity->y + sprite_y;
            int screen_x = entity->x + sprite_x;

            if (screen_y < 0 || screen_y >= game->main_win.rows || screen_x < 0 ||
                screen_x >= game->main_win.cols) {
                continue;
            }

            char sprite_char = sprite_grid[sprite_y * entity->width + sprite_x];

            if (sprite_char != ' ') {
                mvwaddch(win, screen_y, screen_x, sprite_char);
            }
        }
    }
    wnoutrefresh(win);
}

void swap_height_width(entity_t* entity) {
    int buf = entity->width;
    entity->width = entity->height;
    entity->height = buf;
}

void remove_sprite(Game* game, entity_t* entity) {
    WINDOW* win = game->main_win.window;

    for (int sprite_y = 0; sprite_y < entity->height; sprite_y++) {
        for (int sprite_x = 0; sprite_x < entity->width; sprite_x++) {
            int screen_y = entity->y + sprite_y;
            int screen_x = entity->x + sprite_x;

            mvwaddch(win, screen_y, screen_x, ' ');
        }
    }
    wnoutrefresh(win);
}

void redraw_sprite(Game* game, entity_t* entity) {
    remove_sprite(game, entity);
    draw_sprite(game, entity);
    wnoutrefresh(game->main_win.window);
}

void draw_status(Game* game) {
    box(game->status_win.window, 0, 0);
    wattron(game->status_win.window, A_BOLD);
    mvwprintw(game->status_win.window, 1, 2, "Player: Skyblueborb | Level 1 | Life-force: %-3d", game->entities.swallow->hp);
    mvwprintw(game->status_win.window, 2, 2,
              "Stars collected: PL | Star Quota: %d | Time left: %.1f", game->config.star_quota,
              game->config.timer);
    mvwprintw(game->status_win.window, 3, 2, "Game speed: %-3d", game->game_speed);
    wattroff(game->status_win.window, A_BOLD);
    wnoutrefresh(game->status_win.window);
}

void draw_main(Game* game) {
    box(game->main_win.window, 0, 0);
    wnoutrefresh(game->main_win.window);
}
