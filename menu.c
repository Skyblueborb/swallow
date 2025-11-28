#include <unistd.h>
#include "conf.h"
#include "graphics.h"
#include "ranking.h"
#include "star.h"
#include "types.h"
#include "utils.h"

static void show_high_scores(Game* game) {
    WINDOW* win = game->main_win.window;
    RankingNode* scores = load_rankings();
    RankingNode* current = scores;

    wclear(win);
    box(win, 0, 0);
    mvwprintw(win, 1, 2, "HIGH SCORES:");

    int row = 3;
    int index = 1;
    while (current != NULL && row < game->main_win.rows - 1) {
        mvwprintw(win, row, 4, "%d) %d - %s", index, current->score, current->username);
        row++;
        index++;
        current = current->next;
    }

    wrefresh(win);
    free_rankings(scores);

    flushinp();
    wgetch(win);
}

static void draw_menu_stars(Game* game, WINDOW* win) {
    if ((rand() % 5) == 0) {
        spawn_star(game);
    }

    Star* curr = game->entities.stars;
    Star* prev = NULL;

    wattron(win, A_BOLD);

    while (curr != NULL) {
        draw_sprite(game, &curr->ent);

        curr->ent.y += curr->ent.dy;

        if (curr->ent.y >= game->main_win.rows - 1) {
            Star* to_free = curr;
            if (prev == NULL)
                game->entities.stars = curr->next;
            else
                prev->next = curr->next;

            curr = curr->next;
            free(to_free);
        } else {
            prev = curr;
            curr = curr->next;
        }
    }

    wattroff(win, A_BOLD);
}

MenuOption show_start_menu(Game* game) {
    WINDOW* win = game->main_win.window;
    int selection = 0;
    int num_options = 4;
    const char* options[] = {"START GAME", "HIGH SCORES", "CHANGE USERNAME", "EXIT"};

    game->entities.stars = NULL;

    nodelay(win, TRUE);
    keypad(win, TRUE);

    while (1) {
        wclear(win);
        box(win, 0, 0);

        draw_menu_stars(game, win);

        int center_x = game->main_win.cols / 2;
        int art_start_y = 1;
        draw_ascii_art(game, center_x, art_start_y);

        int menu_start_y = art_start_y + 6 + 4;
        for (int i = 0; i < num_options; i++) {
            if (i == selection) {
                wattron(win, A_REVERSE | A_BOLD);
                mvwprintw(win, menu_start_y + (i * 2), center_x - 10, "-> %s", options[i]);
                wattroff(win, A_REVERSE | A_BOLD);
            } else {
                mvwprintw(win, menu_start_y + (i * 2), center_x - 10, "   %s", options[i]);
            }
        }

        if (game->username) {
            mvwprintw(win, game->main_win.rows - 2, 2, "Logged in as: %s", game->username);
        }

        wrefresh(win);

        int c = wgetch(win);

        if (c != ERR) {
            if (c == KEY_UP) {
                selection--;
                if (selection < 0) selection = num_options - 1;
            } else if (c == KEY_DOWN) {
                selection++;
                if (selection >= num_options) selection = 0;
            } else if (c == '\n') {
                break;
            }
        }

        usleep(50000);
    }

    Star* curr = game->entities.stars;
    while (curr) {
        Star* next = curr->next;
        free(curr);
        curr = next;
    }
    game->entities.stars = NULL;

    nodelay(stdscr, TRUE);
    wclear(win);
    wrefresh(win);

    return (MenuOption)selection;
}

void menu_loop(Game* game, MenuOption choice) {
    wclear(game->status_win.window);
    wrefresh(game->status_win.window);
    char* level_path;
    switch (choice) {
        case MENU_START_GAME:
            level_path = select_level(game);
            game->config = read_config(level_path);
            free(level_path);
            delwin(game->main_win.window);
            delwin(game->status_win.window);
            setup_windows(&game->main_win, &game->status_win, &game->config);
            srand(game->config.seed);
            init_occupancy_map(game);

            game->running = 1;
            game->game_speed = 3;
            game->time_left = game->config.timer;
            game->score = 0.f;

            Swallow* swallow = malloc(sizeof(Swallow));
            init_swallow(game, swallow);

            game->entities.swallow = swallow;

            while (game->running) {
                game_loop(game);
            }

            save_ranking(game);
            show_high_scores(game);
            break;
        case MENU_HIGH_SCORES:
            nodelay(game->main_win.window, FALSE);
            show_high_scores(game);
            nodelay(game->main_win.window, TRUE);
            break;
        case MENU_USERNAME:
            nodelay(game->main_win.window, FALSE);
            get_username(game);
            nodelay(game->main_win.window, TRUE);
            break;
        case MENU_EXIT:
            game->menu_running = 0;
            break;
    }
}
