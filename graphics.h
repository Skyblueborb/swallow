#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "types.h"

void draw_sprite(Game *game, entity_t *entity);
void remove_sprite(Game *game, entity_t *entity);

void draw_status(Game *game);
void draw_main(Game *game);
void draw_ascii_art(Game *game, int center_x, int art_start_y);

#endif /* end of include guard: GRAPHICS_H */
