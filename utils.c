#include "utils.h"
#include <dirent.h>
#include <string.h>
#include "types.h"

void strip_newline(char* str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
}

static void init_game_colors() {
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
    keypad(stdscr, TRUE);
    refresh();
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

    wrefresh(main_win->window);
    wrefresh(status_win->window);
}

void setup_menu_window(WIN* menu_win) {
    int rows = 32;
    int cols = 100;
    int x = 0;
    int y = 0;

    if (menu_win->window) {
        wclear(stdscr);
        wrefresh(stdscr);
        delwin(menu_win->window);
    }

    menu_win->window = newwin(rows, cols, y, x);
    menu_win->rows = rows;
    menu_win->cols = cols;
    menu_win->x = x;
    menu_win->y = y;

    wattron(menu_win->window, COLOR_PAIR(C_GREY_1));
    box(menu_win->window, 0, 0);
    wattroff(menu_win->window, COLOR_PAIR(C_GREY_1));

    wrefresh(menu_win->window);
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

static int compare_levels(const void* a, const void* b) {
    return strcmp(*(const char**)a, *(const char**)b);
}

int load_levels(char*** files) {
    DIR* d = opendir("levels");
    int count = 0;
    struct dirent* dir;

    if (!d) return 0;

    while ((dir = readdir(d))) {
        if (strstr(dir->d_name, ".conf")) {
            char** tmp = realloc(*files, sizeof(char*) * (count + 1));
            if (tmp) {
                *files = tmp;
                (*files)[count++] = strdup(dir->d_name);
            }
        }
    }
    closedir(d);

    if (*files && count > 1) {
        qsort(*files, count, sizeof(char*), compare_levels);
    }
    return count;
}
