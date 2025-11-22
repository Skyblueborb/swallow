#include "types.h"

// We assume the fields are empty where we update
void update_occupancy_map(char** occupancy_map, int rows, int cols, entity_t* ent,
                          char map_representation) {
    int x = ent->x;
    int y = ent->y;
    int width = ent->width;
    int height = ent->height;

    for (int i = y; i < y + height; i++) {
        for (int j = x; j < x + width; j++) {
            if (i >= 0 && i < rows && j >= 0 && j < cols) {
                if (occupancy_map[i][j] != WALL) {
                    occupancy_map[i][j] = map_representation;
                }
            }
        }
    }
}

// Returns 0 for good positions, 1 for WALL, 2 for HUNTER, 3 for Swallow
collision_t check_occupancy_map(char** occupancy_map, int rows, int cols, int x, int y, int width,
                        int height) {
    if (x <= 0 || y <= 0 || x + width >= cols - 1 || y + height >= rows) {
        return WALL;  // Collision with Border Walls
    }
    for (int i = y; i < y + height; i++) {
        for (int j = x; j < x + width; j++) {
            if (occupancy_map[i][j] == WALL) {
                return WALL;
            }
            if (occupancy_map[i][j] == HUNTER) {
                return HUNTER;
            }
            if (occupancy_map[i][j] == SWALLOW) {
                return SWALLOW;
            }
        }
    }
    return EMPTY;
}
