#include <string.h>
#include "utils.h"
#include "types.h"

void strip_newline(char* str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
}

void init_game_colors() {
    init_pair(C_RED_1, 52, -1);
    init_pair(C_RED_2, 88, -1);
    init_pair(C_RED_3, 124, -1);
    init_pair(C_RED_4, 160, -1);
    init_pair(C_RED_5, 196, -1);

    init_pair(C_GREEN_1, 22, -1);
    init_pair(C_GREEN_2, 28, -1);
    init_pair(C_GREEN_3, 34, -1);
    init_pair(C_GREEN_4, 40, -1);
    init_pair(C_GREEN_5, 46, -1);

    init_pair(C_BLUE_1, 17, -1);
    init_pair(C_BLUE_2, 21, -1);
    init_pair(C_BLUE_3, 27, -1);
    init_pair(C_BLUE_4, 33, -1);
    init_pair(C_BLUE_5, 51, -1);

    init_pair(C_YELLOW_1, 94, -1);
    init_pair(C_YELLOW_2, 130, -1);
    init_pair(C_YELLOW_3, 172, -1);
    init_pair(C_YELLOW_4, 214, -1);
    init_pair(C_YELLOW_5, 226, -1);

    init_pair(C_PURPLE_1, 53, -1);
    init_pair(C_PURPLE_2, 90, -1);
    init_pair(C_PURPLE_3, 127, -1);
    init_pair(C_PURPLE_4, 163, -1);
    init_pair(C_PURPLE_5, 201, -1);

    init_pair(C_CYAN_1, 23, -1);
    init_pair(C_CYAN_2, 30, -1);
    init_pair(C_CYAN_3, 37, -1);
    init_pair(C_CYAN_4, 44, -1);
    init_pair(C_CYAN_5, 51, -1);

    init_pair(C_GREY_1, 240, -1);
    init_pair(C_GREY_2, 250, -1);

    init_pair(PAIR_PLAYER, 51, -1);

    init_pair(PAIR_STAR, 220, -1);

    init_pair(PAIR_DEFAULT, 255, -1);
}

void init_curses() {
    if (initscr() == NULL) {
        fprintf(stderr, "Failed to initialize ncurses\n");
        exit(1);
    }
    if (!has_colors() && COLORS < 256) {
        endwin();
        printf("Your terminal does not support color\n");
        exit(1);
    }
    start_color();
    use_default_colors();
    init_game_colors();
    noecho();
    curs_set(0);
    nodelay(stdscr, TRUE);
    refresh();
}

void get_username(Game *game) {
    char buffer[50] = {0};
    WINDOW *win = game->main_win.window;
    int rows = game->main_win.rows;
    int cols = game->main_win.cols;

    int center_y = rows / 2;
    int center_x = (cols - 16) / 2;

    mvwprintw(win, center_y, center_x, "Enter Username: ");
    wmove(win, center_y + 1, center_x);
    wrefresh(win);

    nodelay(stdscr, FALSE);
    echo();
    curs_set(1);

    wgetnstr(win, buffer, 49);

    noecho();
    curs_set(0);
    nodelay(stdscr, TRUE);

    strip_newline(buffer);

    if (strlen(buffer) == 0) {
        strcpy(buffer, "Player");
    }

    game->username = malloc(strlen(buffer) + 1);
    if (game->username) {
        strcpy(game->username, buffer);
    } else {
        exit(1);
    }

    wclear(win);
    wrefresh(win);
}

void setup_windows(WIN* main_win, WIN* status_win, const conf_t* config) {
    int game_area_height = config->window_height;
    int game_area_width = config->window_width;

    main_win->rows = 4 * game_area_height / 5;
    main_win->cols = game_area_width;
    main_win->y = 0;
    main_win->x = 0;
    main_win->window = newwin(main_win->rows, main_win->cols, main_win->y, main_win->x);

    status_win->rows = game_area_height - main_win->rows;
    status_win->cols = game_area_width;
    status_win->y = main_win->rows;
    status_win->x = 0;
    status_win->window = newwin(status_win->rows, status_win->cols, status_win->y, status_win->x);

    box(main_win->window, 0, 0);
    box(status_win->window, 0, 0);
    wrefresh(main_win->window);
    wrefresh(status_win->window);
}

void free_occupancy_map(Game* game) {
    if (game->occupancy_map != NULL) {
        int rows = game->main_win.rows;
        for (int y = 0; y < rows; y++) {
            free(game->occupancy_map[y]);
        }
        free(game->occupancy_map);
    }
}

void init_occupancy_map(Game* game) {
    int rows = game->main_win.rows;
    int cols = game->main_win.cols;

    game->occupancy_map = malloc(rows * sizeof(char*));

    if (game->occupancy_map == NULL) {
        endwin();
        fprintf(stderr, "Fatal Error: Failed to allocate memory for map rows.\n");
        exit(1);
    }

    for (int y = 0; y < rows; y++) {
        game->occupancy_map[y] = malloc(cols * sizeof(char));
        if (game->occupancy_map[y] == NULL) {
            endwin();
            fprintf(stderr, "Fatal Error: Failed to allocate memory for map columns.\n");
            free_occupancy_map(game);
            exit(1);
        }
    }

    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            game->occupancy_map[y][x] = ' ';
            if (y == 0 || x == 0 || x == cols - 1 || y == rows - 1) {
                game->occupancy_map[y][x] = WALL;
            } else {
                game->occupancy_map[y][x] = EMPTY;
            }
        }
    }
}

void change_game_speed(Game* game, increment_t increment) {
    int current_speed = game->game_speed;
    if (increment == UP && game->config.max_speed >= current_speed + 1) {
        game->game_speed++;
    } else if (increment == DOWN && game->config.min_speed <= current_speed - 1) {
        game->game_speed--;
    }
}

void init_swallow(Game* game, Swallow* swallow) {
    entity_t* s = &swallow->ent;

    swallow->hp = 100;
    s->width = 3;
    s->height = 3;
    s->x = game->main_win.cols / 2;
    s->y = game->main_win.rows / 2;
    s->speed = 1;
    s->direction = DIR_RIGHT;
    s->color = C_GREEN_5;
    s->sprites[DIR_UP] =
            ".^."
            "/o\\"
            ".Y.";
    s->sprites[DIR_DOWN] =
            "_w_"
            "\\o/"
            ".v.";
    s->sprites[DIR_LEFT] =
            "./."
            "<o="
            ".\\.";
    s->sprites[DIR_RIGHT] =
            ".\\."
            "=o>"
            "./.";
}

int count_hunters(Game* game) {
    int count = 0;
    for (Hunter* h = game->entities.hunters; h; h = h->next) count++;
    return count;
}

void free_hunters(Game* game) {
    Hunter* h = game->entities.hunters;
    while (h != NULL) {
        Hunter* next = h->next;
        free(h);
        h = next;
    }
}
