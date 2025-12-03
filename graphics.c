#include <ncurses.h>
#include <string.h>
#include "types.h"

void draw_sprite(Game* game, entity_t* entity) {
    char* sprite_grid;
    if (entity->anim_frame == 1 && entity->anim_sprites[entity->direction] != NULL) {
        sprite_grid = entity->anim_sprites[entity->direction];
    } else {
        sprite_grid = entity->sprites[entity->direction];
    }

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
    mvwprintw(win, 2, 2, "Stars collected: %-3d | Star Quota: %-3d | Time left: %.1f ",
              game->stars_collected, game->config.star_quota, game->time_left);
    mvwprintw(win, 3, 2, "Game speed: %-3d", game->game_speed);
    mvwprintw(win, 4, 2, "Taxi cooldown: %.1f ", game->albatross_cooldown);
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

void draw_ascii_art(const Game* game, const int center_x, const int art_start_y, const char** ascii_art,
                    const int art_lines, const ColorPair color) {
    WINDOW* win = game->main_win.window;

    wattron(win, COLOR_PAIR(color));
    for (int i = 0; i < art_lines; i++) {
        int len = mbstowcs(NULL, ascii_art[i], 0);
        int x = center_x - (len / 2);
        if (x < 1) x = 1;
        mvwprintw(win, art_start_y + i, x, "%s", ascii_art[i]);
    }
    wattroff(win, COLOR_PAIR(color));
    wrefresh(win);
}

void draw_logo(Game* game, const int center_x, const int art_start_y) {
    const char* logo_art[] = {
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
    const int logo_lines = sizeof(logo_art) / sizeof(logo_art[0]);
    draw_ascii_art(game, center_x, art_start_y, logo_art, logo_lines, PAIR_PLAYER);
}

void draw_game_over(Game* game, const int center_x, const int art_start_y) {
    const char* win_art[] = {
            "          _______                      _______  _        _ ",
            "|\\     /|(  ___  )|\\     /|  |\\     /|(  ___  )( (    /|( )",
            "( \\   / )| (   ) || )   ( |  | )   ( || (   ) ||  \\  ( || |",
            " \\ (_) / | |   | || |   | |  | | _ | || |   | ||   \\ | || |",
            "  \\   /  | |   | || |   | |  | |( )| || |   | || (\\ \\) || |",
            "   ) (   | |   | || |   | |  | || || || |   | || | \\   |(_)",
            "   | |   | (___) || (___) |  | () () || (___) || )  \\  | _ ",
            "   \\_/   (_______)(_______)  (_______)(_______)|/    )_)(_)",
    };

    const char* lose_art[] = {
            "          _______             _        _______  _______  _______          ",
            "|\\     /|(  ___  )|\\     /|  ( \\      (  ___  )(  ____ \\(  ____ \\         ",
            "( \\   / )| (   ) || )   ( |  | (      | (   ) || (    \\/| (    \\/         ",
            " \\ (_) / | |   | || |   | |  | |      | |   | || (_____ | (__             ",
            "  \\   /  | |   | || |   | |  | |      | |   | |(_____  )|  __)            ",
            "   ) (   | |   | || |   | |  | |      | |   | |      ) || (               ",
            "   | |   | (___) || (___) |  | (____/\\| (___) |/\\____) || (____/\\  _  _  _ ",
            "   \\_/   (_______)(_______)  (_______/(_______)\\_______)(_______/ (_)(_)(_)"};

    if (game->result == WINNER) {
        const int art_lines = sizeof(win_art) / sizeof(win_art[0]);
        draw_ascii_art(game, center_x, art_start_y, win_art, art_lines, C_GREEN_5);
    } else {
        const int art_lines = sizeof(lose_art) / sizeof(lose_art[0]);
        draw_ascii_art(game, center_x, art_start_y, lose_art, art_lines, C_RED_5);
    }
}

void draw_high_scores(Game* game, const int center_x, const int art_start_y) {
    const char* high_score_art[] = {
            "▗▖ ▗▖▗▄▄▄▖ ▗▄▄▖▗▖ ▗▖     ▗▄▄▖ ▗▄▄▖ ▗▄▖ ▗▄▄▖ ▗▄▄▄▖ ▗▄▄▖   ",
            "▐▌ ▐▌  █  ▐▌   ▐▌ ▐▌    ▐▌   ▐▌   ▐▌ ▐▌▐▌ ▐▌▐▌   ▐▌     ▀",
            "▐▛▀▜▌  █  ▐▌▝▜▌▐▛▀▜▌     ▝▀▚▖▐▌   ▐▌ ▐▌▐▛▀▚▖▐▛▀▀▘ ▝▀▚▖  ▄",
            "▐▌ ▐▌▗▄█▄▖▝▚▄▞▘▐▌ ▐▌    ▗▄▄▞▘▝▚▄▄▖▝▚▄▞▘▐▌ ▐▌▐▙▄▄▖▗▄▄▞▘   ",
    };
    const int art_lines = sizeof(high_score_art) / sizeof(high_score_art[0]);
    draw_ascii_art(game, center_x, art_start_y, high_score_art, art_lines, C_YELLOW_5);
}
