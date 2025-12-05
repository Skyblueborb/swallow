#ifndef TYPES_H
#define TYPES_H

#include <ncurses.h>

#define MAX_SAFE_ZONE_ATTEMPTS 50

#define ASCII_LOGO_LINES 6
#define ASCII_WIN_LOOSE_LINES 8
#define ASCII_HIGH_SCORE_LINES 4

#define HUNTER_IDLE_TICKS 30
#define HUNTER_DASH_COOLDOWN_TICKS 100

#define HUNTER_ESCALATION_FREQUENCY 5.0f

#define MAX_USERNAME_LENGTH 50

#define MENU_ROWS 32
#define MENU_COLS 100
#define MENU_X 0
#define MENU_Y 0

#define PHYSICS_TOUCHING_TOLERANCE 1

#define SWALLOW_HP 100
#define SWALLOW_SIZE 3
#define SWALLOW_SPEED 1

#define STAR_MOVE_TICKS 4
#define STAR_SPEED_MAX 3

#define GAME_OVER_INPUT_BLOCK 2000000

#define BASE_SPAWNER_MULTIPLIER 10

#define ANIMATION_TICKS 5

#define TAXI_ANIMATION_TICK_SPEED 30000
#define ALBATROSS_TAXI_DURATION 20.0f
#define ALBATROSS_TAXI_FRAMES 20
#define SAFE_ZONE_PADDING 10

#define MENU_STAR_AMOUNT 5

#define CENTER_Y_OFFSET 10
#define CENTER_X_OFFSET 10

#define LOGO_START 1

#define MAX_LINE_LENGTH 256

#define MAX_REDUCTION_FACTOR 0.2f

#define MAX_SPAWN_HUNTER_THRESHOLD 5

#define GAME_OVER_HIGH_SCORE_Y_OFFSET 10

#define MENU_TICK_SPEED 50000

#define LEVEL_SELECT_X_OFFSET 20

#define REPLAY_CHUNK 512

typedef struct {
    WINDOW* window;
    int x, y, rows, cols;
} WIN;

typedef enum { DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT, NUM_DIRECTIONS } direction_t;

typedef enum { UP, DOWN } increment_t;

typedef enum { WALL, HUNTER, SWALLOW, STAR, EMPTY } collision_t;

typedef enum { UNKNOWN, WINNER, LOSER } result_t;

typedef enum {
    PAIR_DEFAULT = 1,
    PAIR_PLAYER,

    C_RED_1,
    C_RED_2,
    C_RED_3,
    C_RED_4,
    C_RED_5,
    C_GREEN_1,
    C_GREEN_2,
    C_GREEN_3,
    C_GREEN_4,
    C_GREEN_5,
    C_BLUE_1,
    C_BLUE_2,
    C_BLUE_3,
    C_BLUE_4,
    C_BLUE_5,
    C_YELLOW_1,
    C_YELLOW_2,
    C_YELLOW_3,
    C_YELLOW_4,
    C_YELLOW_5,
    C_PURPLE_1,
    C_PURPLE_2,
    C_PURPLE_3,
    C_PURPLE_4,
    C_PURPLE_5,
    C_CYAN_1,
    C_CYAN_2,
    C_CYAN_3,
    C_CYAN_4,
    C_CYAN_5,
    C_GREY_1,
    C_GREY_2
} ColorPair;

typedef enum {
    REPLAY_RECORDING,
    REPLAY_PLAYING
} ReplayState;

// Entity types
typedef struct {
    int x, y;
    int dx, dy;
    int speed;
    int height, width;
    char* sprites[NUM_DIRECTIONS];
    char* anim_sprites[NUM_DIRECTIONS];
    int anim_frame;
    int anim_timer;
    direction_t direction;
    ColorPair color;
} entity_t;

typedef struct {
    entity_t ent;
    int hp;
} Swallow;

typedef struct Star {
    entity_t ent;
    struct Star* next;
} Star;

// Hunter Types
typedef enum { HUNTER_IDLE, HUNTER_PAUSED, HUNTER_DASHING } HunterState;

typedef struct Hunter {
    entity_t ent;
    int bounces;
    int damage;
    HunterState state;
    int state_timer;
    int base_speed;
    int dash_cooldown;
    struct Hunter* next;
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
    int min_speed;
    int max_speed;
    int seed;
    int hunter_templates_amount;
    float timer;
    float star_spawn;
    float hunter_spawn;
    float score_time_weight;
    float score_stars_weight;
    float score_life_weight;
    float albatross_cooldown;
    float hunter_spawn_esc;
    float hunter_bounce_esc;
    HunterTypes* hunter_templates;
} conf_t;

typedef enum { TYPE_INT, TYPE_FLOAT, TYPE_STRING, TYPE_COLOR } ConfigType;

typedef struct {
    const char* key_name;
    size_t offset;
    ConfigType type;
} ConfigMapEntry;

typedef struct {
    const char* name;
    ColorPair value;
} ColorNameEntry;

typedef struct GameEntities {
    Swallow* swallow;
    Hunter* hunters;
    Star* stars;
} GameEntities;

typedef struct {
    ReplayState replay_state;
    char* replay_keys;
    char* replay_level_name;
    unsigned int replay_index;
    unsigned int replay_chunks;
    unsigned int playback_index;
} replay_t;

typedef struct {
    conf_t config;
    WIN main_win;
    WIN status_win;
    char running;
    char menu_running;
    replay_t replay;
    char result;
    char** occupancy_map;
    char* username;
    float time_left;
    float albatross_cooldown;
    int game_speed;
    int stars_collected;
    int hunter_spawn_tick;
    int star_spawn_tick;
    int star_move_tick;
    int star_flicker_tick;
    int score;
    GameEntities entities;
} Game;

typedef struct RankingNode {
    int score;
    char username[MAX_USERNAME_LENGTH];
    struct RankingNode* next;
} RankingNode;

typedef enum { MENU_START_GAME, MENU_REPLAY, MENU_HIGH_SCORES, MENU_USERNAME, MENU_EXIT } MenuOption;

#endif  // TYPES_H
