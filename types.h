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

// Entity types
typedef struct {
    int x, y;
    int dx, dy;
    int speed;
    int height, width;
    char* sprites[NUM_DIRECTIONS];
    direction_t direction;
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
} HunterTypes;

// Config types
typedef struct {
    int window_height;
    int window_width;
    int star_quota;
    float timer;
    float star_spawn;
    float hunter_spawn;
    int min_speed;
    int max_speed;
    HunterTypes hunter_templates[5];
} conf_t;

typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_STRING,
} ConfigType;

typedef struct {
    const char *key_name;
    size_t offset;
    ConfigType type;
} ConfigMapEntry;

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
    float time_left;
    int game_speed;
    int stars_collected;
    GameEntities entities;
} Game;

#endif // TYPES_H
