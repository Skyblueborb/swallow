#include "utils.h"
#include "types.h"

void init_curses() {
    if (initscr() == NULL) {
        fprintf(stderr, "Failed to initialize ncurses\n");
        exit(1);
    }
    if (!has_colors()) {
        endwin();
        printf("Your terminal does not support color\n");
        exit(1);
    }
    start_color();
    noecho();
    curs_set(0);
    nodelay(stdscr, TRUE);
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
