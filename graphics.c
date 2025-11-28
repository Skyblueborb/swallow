#include <ncurses.h>
#include <string.h>
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
    mvwprintw(win, 1, 2, "Player: %s | Level %-2d | Life-force: %-3d", game->username,
              game->config.level_nr, game->entities.swallow->hp);
    mvwprintw(win, 2, 2, "Stars collected: %-3d | Star Quota: %-3d | Time left: %.1f",
              game->stars_collected, game->config.star_quota, game->time_left);
    mvwprintw(win, 3, 2, "Game speed: %-3d", game->game_speed);
    mvwprintw(win, 4, 2, "Taxi cooldown: %-3d",
              game->albatross_cooldown > 0 ? game->albatross_cooldown : 0);
    mvwprintw(win, 5, 2, "Score: %-10d", (int)game->score);
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

void draw_ascii_art(Game* game, int center_x, int art_start_y) {
    WINDOW* win = game->main_win.window;
    const char* ascii_art[] = {
            "  _________               .__  .__                    _________ __                    "
            "   ",
            " /   _____/_  _  _______  |  | |  |   ______  _  __  /   _____//  |______ _______  "
            "______",
            " \\_____  \\\\ \\/ \\/ /\\__  \\ |  | |  |  /  _ \\ \\/ \\/ /  \\_____  \\\\   __\\__ "
            " \\\\_  __ \\/  ___/",
            " /        \\\\     /  / __ \\|  |_|  |_(  <_> )     /   /        \\|  |  / __ \\|  | "
            "\\/\\___ \\ ",
            "/_______  / \\/\\_/  (____  /____/____/\\____/ \\/\\_/   /_______  /|__| (____  /__|  "
            "/____  >",
            "        \\/              \\/                                  \\/           \\/       "
            "    \\/"};
    int art_lines = 6;

    if (art_start_y < 1) art_start_y = 1;

    wattron(win, COLOR_PAIR(PAIR_PLAYER) | A_BOLD);
    for (int i = 0; i < art_lines; i++) {
        int len = strlen(ascii_art[i]);
        int x = center_x - (len / 2);
        if (x < 1) x = 1;
        mvwprintw(win, art_start_y + i, x, "%s", ascii_art[i]);
    }
    wattroff(win, COLOR_PAIR(PAIR_PLAYER) | A_BOLD);
}
