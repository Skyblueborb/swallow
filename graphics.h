#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "types.h"

void redraw_sprite(Game *game, entity_t *entity);
void remove_sprite(Game *game, entity_t *entity);
void draw_sprite(Game *game, entity_t *entity);
void draw_status(Game *game);
void draw_main(Game *game);
void swap_height_width(entity_t *entity);

#endif /* end of include guard: GRAPHICS_H */
