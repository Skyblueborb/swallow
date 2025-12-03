#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "types.h"

void draw_sprite(Game *game, entity_t *entity);
void remove_sprite(Game *game, entity_t *entity);

void draw_status(Game *game);
void draw_main(Game *game);
void draw_ascii_art(Game* game, const int center_x, const int art_start_y, const char** ascii_art, const int art_lines);
void draw_logo(Game *game, const int center_x, const int art_start_y);
void draw_game_over(Game *game, const int center_x, const int art_start_y);
void draw_high_scores(Game* game, const int center_x, const int art_start_y);

#endif /* end of include guard: GRAPHICS_H */
