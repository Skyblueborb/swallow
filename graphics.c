#include "types.h"

void draw_sprite(Game* game, entity_t* entity) {
    char* sprite_grid = entity->sprites[entity->direction];
    WINDOW* win = game->main_win.window;

    if (entity->color) {
        wattron(win, COLOR_PAIR(entity->color));
    }

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
    if (entity->color) {
        wattroff(win, COLOR_PAIR(entity->color));
    }
    wnoutrefresh(win);
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

void draw_status(Game* game) {
    WINDOW* win = game->status_win.window;
    wattron(win, COLOR_PAIR(C_GREY_1));
    box(win, 0, 0);
    wattroff(win, COLOR_PAIR(C_GREY_1));
    wattron(win, A_BOLD);
    mvwprintw(win, 1, 2, "Player: %s | Level %-2d | Life-force: %-3d", game->username, game->config.level_nr, game->entities.swallow->hp);
    mvwprintw(win, 2, 2, "Stars collected: %-3d | Star Quota: %-3d | Time left: %.1f",
              game->stars_collected, game->config.star_quota, game->time_left);
    mvwprintw(win, 3, 2, "Game speed: %-3d", game->game_speed);
    wattroff(win, A_BOLD);
    wnoutrefresh(win);
}

void draw_main(Game* game) {
    WINDOW* win = game->main_win.window;
    wattron(win, COLOR_PAIR(C_GREY_1));
    box(win, 0, 0);
    wattroff(win, COLOR_PAIR(C_GREY_1));
    wnoutrefresh(win);
}
