#include "types.h"

void update_occupancy_map(char **occupancy_map, int rows, int cols, entity_t *ent, char map_representation);
collision_t check_occupancy_map(char** occupancy_map, int rows, int cols, int x, int y, int width,int height);
