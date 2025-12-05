#include <locale.h>
#include <ncurses.h>
#include <stdlib.h>

#include "conf.h"
#include "hunter.h"
#include "menu.h"
#include "star.h"
#include "types.h"
#include "utils.h"

int main() {
    setlocale(LC_ALL, "");
    Game game = {0};

    init_curses();

    setup_menu_window(&game.main_win);

    get_username(&game);

    game.menu_running = 1;
    while (game.menu_running) {
        setup_menu_window(&game.main_win);
        const MenuOption choice = show_start_menu(&game);
        handle_menu_choice(&game, choice);
    }

    if (game.entities.hunters) {
        free_hunters(&game);
    }
    if (game.entities.stars) {
        free_stars(&game);
    }
    if (game.entities.swallow) {
        free(game.entities.swallow);
    }

    delwin(game.main_win.window);
    delwin(game.status_win.window);
    endwin();

    if (game.username) {
        free(game.username);
    }
    free_config(&game.config);

    if (game.replay.replay_keys != NULL) {
        free(game.replay.replay_level_name);
        free(game.replay.replay_keys);
    }

    return 0;
}
