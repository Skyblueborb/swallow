#ifndef TYPES_H
#define TYPES_H

#include <curses.h>
#include <stdlib.h>
#include <inttypes.h>

typedef struct {
    WINDOW* window;
    int x, y, rows, cols;
} WIN;

typedef enum {
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT,
    NUM_DIRECTIONS
} direction_t;

typedef enum {
    UP,
    DOWN
} increment_t;

typedef enum {
    WALL,
    HUNTER,
    SWALLOW,
    STAR,
    EMPTY
} collision_t;

typedef enum {
    PAIR_DEFAULT = 1,
    PAIR_PLAYER,
    PAIR_STAR,

    C_RED_1, C_RED_2, C_RED_3, C_RED_4, C_RED_5,
    C_GREEN_1, C_GREEN_2, C_GREEN_3, C_GREEN_4, C_GREEN_5,
    C_BLUE_1, C_BLUE_2, C_BLUE_3, C_BLUE_4, C_BLUE_5,
    C_YELLOW_1, C_YELLOW_2, C_YELLOW_3, C_YELLOW_4, C_YELLOW_5,
    C_PURPLE_1, C_PURPLE_2, C_PURPLE_3, C_PURPLE_4, C_PURPLE_5,
    C_CYAN_1, C_CYAN_2, C_CYAN_3, C_CYAN_4, C_CYAN_5,
    C_GREY_1, C_GREY_2
} ColorPair;

// Entity types
typedef struct {
    int x, y;
    int dx, dy;
    int speed;
    int height, width;
    char* sprites[NUM_DIRECTIONS];
    direction_t direction;
    ColorPair color;
} entity_t;

typedef struct {
    entity_t ent;
    int hp;
} Swallow;

typedef struct Star {
    entity_t ent;
    struct Star *next;
} Star;

// Hunter Types
typedef struct Hunter {
    entity_t ent;
    int bounces;
    int damage;
    struct Hunter *next;
} Hunter;

typedef struct HunterTypes {
    int width, height;
    int bounces;
    int speed;
    int damage;
    char* sprites[NUM_DIRECTIONS];
    ColorPair color;
} HunterTypes;

// Config types
typedef struct {
    int level_nr;
    int window_height;
    int window_width;
    int star_quota;
    float timer;
    float star_spawn;
    float hunter_spawn;
    int min_speed;
    int max_speed;
    int seed;
    HunterTypes hunter_templates[5];
} conf_t;

typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_STRING,
    TYPE_COLOR
} ConfigType;

typedef struct {
    const char *key_name;
    size_t offset;
    ConfigType type;
} ConfigMapEntry;

typedef struct {
    const char* name;
    ColorPair value;
} ColorNameEntry;

typedef struct GameEntities {
    Swallow *swallow;
    Hunter *hunters;
    Star *stars;
} GameEntities;

typedef struct {
    conf_t config;
    WIN main_win;
    WIN status_win;
    char running;
    char **occupancy_map;
    char *username;
    float time_left;
    int game_speed;
    int stars_collected;
    GameEntities entities;
} Game;

#endif // TYPES_H
