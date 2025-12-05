#include <stdlib.h>

#include "types.h"

void save_key(Game* game, char ch) {
    size_t current_capacity = game->replay.replay_chunks * REPLAY_CHUNK;

    if (game->replay.replay_keys == NULL) {
        game->replay.replay_keys = (char*)calloc(REPLAY_CHUNK, sizeof(char));
        game->replay.replay_index = 0;
        game->replay.replay_chunks = 1;
    } else if (game->replay.replay_index + 1 >= current_capacity) {
        game->replay.replay_chunks++;
        size_t new_size = game->replay.replay_chunks * REPLAY_CHUNK;

        char* new_ptr = (char*)realloc(game->replay.replay_keys, new_size);
        if (new_ptr == NULL) exit(1);

        game->replay.replay_keys = new_ptr;
    }
    game->replay.replay_keys[game->replay.replay_index] = ch;
    game->replay.replay_index++;

    game->replay.replay_keys[game->replay.replay_index] = '\0';
}
