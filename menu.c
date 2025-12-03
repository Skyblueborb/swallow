#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "game.h"
#include "graphics.h"
#include "ranking.h"
#include "star.h"
#include "types.h"
#include "utils.h"

#define MENU_STAR_AMOUNT 5

#define CENTER_Y_OFFSET 10

void show_high_scores(Game* game, const int row_start) {
    WINDOW* win = game->main_win.window;
    RankingNode* scores = load_rankings();
    RankingNode* current = scores;
    nodelay(win, FALSE);

    draw_main(game);
    const int center_x = game->main_win.cols / 2;
    draw_high_scores(game, center_x, row_start);

    int row = row_start + ASCII_HIGH_SCORE_LINES;
    int index = 1;

    if (current == NULL) {
        const char* message = "So lonely here, go play the game!";
        mvwprintw(win, row, center_x - strlen(message) / 2, "%s", message);
    }

    while (current != NULL && row < game->main_win.rows - 1) {
        char* buf;
        asprintf(&buf, "%d) %d - %s", index, current->score, current->username);
        const int length = strlen(buf);
        mvwprintw(win, row, center_x - (length / 2), "%s", buf);
        free(buf);
        row++;
        index++;
        current = current->next;
    }

    wrefresh(win);
    free_rankings(scores);

    flushinp();
    wgetch(win);

    nodelay(win, TRUE);
}

static void draw_menu_stars(Game* game, WINDOW* win) {
    if ((rand() % MENU_STAR_AMOUNT) == 0) {
        spawn_star(game);
    }

    Star* curr = game->entities.stars;
    Star* prev = NULL;

    wattron(win, A_BOLD);

    while (curr != NULL) {
        remove_sprite(game, &curr->ent);

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
            draw_sprite(game, &curr->ent);
            prev = curr;
            curr = curr->next;
        }
    }

    wattroff(win, A_BOLD);
}

static void cleanup_menu_stars(Game* game) {
    Star* curr = game->entities.stars;
    while (curr) {
        Star* next = curr->next;
        free(curr);
        curr = next;
    }
    game->entities.stars = NULL;
}

static void draw_menu_options(WINDOW* win, const int selection, const int num_options,
                              const char** options, const int start_y, const int center_x) {
    for (int i = 0; i < num_options; i++) {
        if (i == selection) {
            wattron(win, A_REVERSE | A_BOLD);
            mvwprintw(win, start_y + (i * 2), center_x - 10, "-> %s", options[i]);
            wattroff(win, A_REVERSE | A_BOLD);
        } else {
            mvwprintw(win, start_y + (i * 2), center_x - 10, "   %s", options[i]);
        }
    }
}

static int handle_menu_input(WINDOW* win, int* const selection, const int num_options) {
    const int c = wgetch(win);
    if (c == ERR) return 0;

    if (c == KEY_UP) {
        (*selection)--;
        if (*selection < 0) *selection = num_options - 1;
    } else if (c == KEY_DOWN) {
        (*selection)++;
        if (*selection >= num_options) *selection = 0;
    } else if (c == '\n') {
        return 1;
    }
    return 0;
}

MenuOption show_start_menu(Game* game) {
    WINDOW* win = game->main_win.window;
    int selection = 0;
    const char* options[] = {"START GAME", "HIGH SCORES", "CHANGE USERNAME", "EXIT"};
    const int num_options = sizeof(options) / sizeof(options[0]);

    game->entities.stars = NULL;

    nodelay(win, TRUE);
    keypad(win, TRUE);

    while (1) {
        draw_main(game);

        draw_menu_stars(game, win);

        const int center_x = game->main_win.cols / 2;
        const int art_start_y = 1;
        draw_logo(game, center_x, art_start_y);

        const int menu_start_y = art_start_y + 10;
        draw_menu_options(win, selection, num_options, options, menu_start_y, center_x);

        if (game->username) {
            mvwprintw(win, game->main_win.rows - 2, 2, "Logged in as: %s", game->username);
        }

        wrefresh(win);

        if (handle_menu_input(win, &selection, num_options)) {
            break;
        }

        usleep(50000);
    }

    cleanup_menu_stars(game);

    nodelay(stdscr, TRUE);
    wclear(win);
    wrefresh(win);

    return (MenuOption)selection;
}

void get_username(Game* game) {
    char buffer[MAX_USERNAME_LENGTH] = {0};
    WINDOW* win = game->main_win.window;
    const int rows = game->main_win.rows;
    const int cols = game->main_win.cols;

    // strlen("Enter Username:") = 16
    const int center_x = (cols - 16) / 2;
    const int center_y = rows / 2 - CENTER_Y_OFFSET;

    draw_main(game);
    mvwprintw(win, center_y, center_x, "Enter Username: ");
    wmove(win, center_y + 1, center_x);
    wrefresh(win);

    nodelay(win, FALSE);
    echo();
    curs_set(1);

    wgetnstr(win, buffer, MAX_USERNAME_LENGTH - 1);

    noecho();
    curs_set(0);
    nodelay(win, TRUE);

    strip_newline(buffer);

    if (strlen(buffer) == 0) {
        strcpy(buffer, "Player");
    }

    char* new_ptr = (char*)realloc(game->username, strlen(buffer) + 1);
    if (new_ptr) {
        game->username = new_ptr;
        strcpy(game->username, buffer);
    } else {
        exit(1);
    }

    wclear(win);
    wrefresh(win);
}

char* select_level(Game* game) {
    char** files = NULL;
    const int count = load_levels(&files);

    if (count == 0) {
        if (files) free(files);
        return strdup("config.txt");
    }

    WINDOW* win = game->main_win.window;
    int sel = 0, c, cx = (game->main_win.cols - 20) / 2;

    nodelay(game->main_win.window, FALSE);
    keypad(win, TRUE);

    while (1) {
        wclear(win);
        wattron(win, COLOR_PAIR(C_GREY_1));
        box(win, 0, 0);
        wattroff(win, COLOR_PAIR(C_GREY_1));
        mvwprintw(win, 2, cx, "SELECT LEVEL:");

        for (int i = 0; i < count; i++) {
            if (i == sel) wattron(win, A_REVERSE);
            mvwprintw(win, 4 + i, cx, "%s", files[i]);
            if (i == sel) wattroff(win, A_REVERSE);
        }
        wrefresh(win);

        c = wgetch(win);
        if (c == KEY_UP)
            sel = (sel - 1 + count) % count;
        else if (c == KEY_DOWN)
            sel = (sel + 1) % count;
        else if (c == '\n')
            break;
    }

    char* res;
    asprintf(&res, "levels/%s", files[sel]);

    for (int i = 0; i < count; i++) free(files[i]);
    free(files);

    nodelay(game->main_win.window, TRUE);
    keypad(win, FALSE);
    wclear(win);
    wrefresh(win);

    return res;
}

void handle_menu_choice(Game* game, const MenuOption choice) {
    switch (choice) {
        case MENU_START_GAME:
            start_game(game);
            end_game(game);
            break;
        case MENU_HIGH_SCORES:
            show_high_scores(game, 1);
            break;
        case MENU_USERNAME:
            get_username(game);
            break;
        case MENU_EXIT:
            game->menu_running = 0;
            break;
    }
}
