#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "conf.h"
#include "types.h"
#include "utils.h"

/**
 * get_*_map()
 * @count: pointer to hold the amount of entries in a map.
 *
 * These maps are used for easy generic parsing of values in parse_values().
 *
 * RETURNS
 * Pointer to static const array containing the map data.
 */
static const ColorNameEntry* get_color_map(int* count) {
    static const ColorNameEntry map[] = {
            {"default", PAIR_DEFAULT}, {"player", PAIR_PLAYER},  {"red_1", C_RED_1},
            {"red_2", C_RED_2},        {"red_3", C_RED_3},       {"red_4", C_RED_4},
            {"red_5", C_RED_5},        {"green_1", C_GREEN_1},   {"green_2", C_GREEN_2},
            {"green_3", C_GREEN_3},    {"green_4", C_GREEN_4},   {"green_5", C_GREEN_5},
            {"blue_1", C_BLUE_1},      {"blue_2", C_BLUE_2},     {"blue_3", C_BLUE_3},
            {"blue_4", C_BLUE_4},      {"blue_5", C_BLUE_5},     {"yellow_1", C_YELLOW_1},
            {"yellow_2", C_YELLOW_2},  {"yellow_3", C_YELLOW_3}, {"yellow_4", C_YELLOW_4},
            {"yellow_5", C_YELLOW_5},  {"purple_1", C_PURPLE_1}, {"purple_2", C_PURPLE_2},
            {"purple_3", C_PURPLE_3},  {"purple_4", C_PURPLE_4}, {"purple_5", C_PURPLE_5},
            {"cyan_1", C_CYAN_1},      {"cyan_2", C_CYAN_2},     {"cyan_3", C_CYAN_3},
            {"cyan_4", C_CYAN_4},      {"cyan_5", C_CYAN_5},     {"grey_1", C_GREY_1},
            {"grey_2", C_GREY_2},
    };
    *count = sizeof(map) / sizeof(map[0]);
    return map;
}

static const ConfigMapEntry* get_global_key_map(int* count) {
    static const ConfigMapEntry map[] = {
            {"level_nr", offsetof(conf_t, level_nr), TYPE_INT},
            {"window_height", offsetof(conf_t, window_height), TYPE_INT},
            {"window_width", offsetof(conf_t, window_width), TYPE_INT},
            {"star_quota", offsetof(conf_t, star_quota), TYPE_INT},
            {"timer", offsetof(conf_t, timer), TYPE_FLOAT},
            {"star_spawn", offsetof(conf_t, star_spawn), TYPE_FLOAT},
            {"hunter_spawn", offsetof(conf_t, hunter_spawn), TYPE_FLOAT},
            {"min_speed", offsetof(conf_t, min_speed), TYPE_INT},
            {"max_speed", offsetof(conf_t, max_speed), TYPE_INT},
            {"seed", offsetof(conf_t, seed), TYPE_INT},
            {"score_time_weight", offsetof(conf_t, score_time_weight), TYPE_FLOAT},
            {"score_stars_weight", offsetof(conf_t, score_stars_weight), TYPE_FLOAT},
            {"score_life_weight", offsetof(conf_t, score_life_weight), TYPE_FLOAT},
            {"albatross_cooldown", offsetof(conf_t, albatross_cooldown), TYPE_FLOAT},
            {"hunter_spawn_esc", offsetof(conf_t, hunter_spawn_esc), TYPE_FLOAT},
            {"hunter_bounce_esc", offsetof(conf_t, hunter_bounce_esc), TYPE_FLOAT}};
    *count = sizeof(map) / sizeof(map[0]);
    return map;
}

static const ConfigMapEntry* get_hunter_key_map(int* count) {
    static const ConfigMapEntry map[] = {
            {"width", offsetof(HunterTypes, width), TYPE_INT},
            {"height", offsetof(HunterTypes, height), TYPE_INT},
            {"bounces", offsetof(HunterTypes, bounces), TYPE_INT},
            {"speed", offsetof(HunterTypes, speed), TYPE_INT},
            {"damage", offsetof(HunterTypes, damage), TYPE_INT},
            {"color", offsetof(HunterTypes, color), TYPE_COLOR},
    };
    *count = sizeof(map) / sizeof(map[0]);
    return map;
}

/**
 * parse_sprite - parses and assigns sprite data to a hunter template
 * @config: pointer to the main config struct
 * @hunter_idx: index of the current hunter template
 * @key: configuration key string (e.g., "sprite_up")
 * @value: the sprite ASCII string
 *
 * This function allocates memory for sprites if not already done (based on
 * width/height), determines the direction based on the key suffix (u/d/l/r),
 * and copies the value into the appropriate slot.
 *
 * RETURNS
 * Void.
 */
static void parse_sprite(conf_t* config, const int hunter_idx, const char* key, const char* value) {
    if (hunter_idx < 0 || !config->hunter_templates) return;
    HunterTypes* const hunter = &config->hunter_templates[hunter_idx];
    if (hunter->width <= 0 || hunter->height <= 0) return;

    const size_t size = hunter->width * hunter->height + 1;
    if (hunter->sprites[0] == NULL) {
        for (int i = 0; i < NUM_DIRECTIONS; i++) {
            hunter->sprites[i] = (char*)malloc(size);
        }
    }

    int dir = -1;
    // Seventh char always contains the direction:
    // sprite_up
    //        ^
    switch (key[7]) {
        case 'u':
            dir = DIR_UP;
            break;
        case 'd':
            dir = DIR_DOWN;
            break;
        case 'l':
            dir = DIR_LEFT;
            break;
        case 'r':
            dir = DIR_RIGHT;
            break;
    }

    if (dir != -1 && hunter->sprites[dir] != NULL) {
        strncpy(hunter->sprites[dir], value, size - 1);
        hunter->sprites[dir][size - 1] = '\0';
    }
}

/**
 * parse_color_name - converts a color string to an enum value
 * @value: string representation of the color (e.g., "red_5")
 *
 * Searches the static color map for a matching name.
 *
 * RETURNS
 * The corresponding integer color pair ID, or PAIR_DEFAULT if not found.
 */
static ColorPair parse_color_name(const char* value) {
    int count = 0;
    const ColorNameEntry* map = get_color_map(&count);

    for (int i = 0; i < count; i++) {
        if (strcmp(value, map[i].name) == 0) {
            return map[i].value;
        }
    }
    return PAIR_DEFAULT;
}

/**
 * parse_values - parses generic key-value pairs
 * @config: pointer to the main configuration struct
 * @hunter_idx: index of the current hunter template
 * @key: key from the config file
 * @value: corresponding value from the config file
 *
 * Deletages parsing for special types such as sprites and colors.
 * For other regular keys, it looks up the target member offset and type in the appropriate map,
 * converts the string value, and writes it to the configuration struct.
 *
 * RETURNS
 * Void.
 */
static void parse_values(conf_t* config, const int hunter_idx, const char* key, const char* value) {
    void* base_ptr = NULL;
    const ConfigMapEntry* map = NULL;
    int count = 0;

    if (hunter_idx < 0) {
        base_ptr = config;
        map = get_global_key_map(&count);
    } else {
        if (strncmp(key, "sprite", strlen("sprite")) == 0) {
            parse_sprite(config, hunter_idx, key, value);
            return;
        }
        base_ptr = &config->hunter_templates[hunter_idx];
        map = get_hunter_key_map(&count);
    }

    for (int i = 0; i < count; i++) {
        if (strcmp(key, map[i].key_name) == 0) {
            void* target = (char*)base_ptr + map[i].offset;

            if (map[i].type == TYPE_INT) {
                *(int*)target = atoi(value);
            } else if (map[i].type == TYPE_FLOAT) {
                *(float*)target = (float)atof(value);
            } else if (map[i].type == TYPE_COLOR) {
                *(ColorPair*)target = parse_color_name(value);
            }
        }
    }
}

/**
 * proccess_config_line - splits config line into key/value pairs
 * @line: entire line read from the config file
 * @config: pointer to the main configuration struct
 * @hunter_idx: index of the current hunter template
 *
 * RETURNS
 * Void.
 */
static void process_config_line(char* line, conf_t* config, int* hunter_idx) {
    const char* key = strtok(line, " \t");
    if (!key || key[0] == '#' || key[0] == '\0') return;

    const char* value = strtok(NULL, "");
    if (value) {
        while (*value && isspace((unsigned char)*value)) value++;
    } else {
        value = "";
    }

    if (strcmp(key, "hunter_template") == 0) {
        (*hunter_idx)++;
        config->hunter_templates_amount = (*hunter_idx) + 1;

        HunterTypes* const temp = (HunterTypes*)realloc(
                config->hunter_templates, config->hunter_templates_amount * sizeof(HunterTypes));
        if (!temp) exit(1);
        config->hunter_templates = temp;

        memset(&config->hunter_templates[*hunter_idx], 0, sizeof(HunterTypes));
    } else {
        parse_values(config, *hunter_idx, key, value);
    }
}

static void init_default_conf(conf_t* config) {
    config->level_nr = 1;
    config->window_height = 40;
    config->window_width = 80;
    config->star_quota = 10;
    config->timer = 50.0f;
    config->star_quota = 3;
    config->hunter_spawn = 12.0f;
    config->min_speed = 1;
    config->max_speed = 5;
    config->seed = 2137;
    config->score_time_weight = 20.0f;
    config->score_stars_weight = 200.0f;
    config->score_life_weight = 5.0f;
    config->albatross_cooldown = 15;
    config->hunter_spawn_esc = 0.05f;
    config->hunter_bounce_esc = 5.0f;
}

conf_t read_config(const char* filename) {
    conf_t config = {0};
    init_default_conf(&config);
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Error opening config file");
        return config;
    }

    // Hunter template initially is -1.
    // When a `hunter_template` is detected we stop looking
    // for global keys and we parse only hunter templates.
    int hunter_index = -1;
    char line[MAX_LINE_LENGTH] = {0};

    while (fgets(line, sizeof(line), file)) {
        strip_newline(line);

        process_config_line(line, &config, &hunter_index);
    }
    fclose(file);
    return config;
}

void free_config(conf_t* config) {
    if (config->hunter_templates == NULL) {
        return;
    }

    for (int i = 0; i < config->hunter_templates_amount; i++) {
        for (int j = 0; j < NUM_DIRECTIONS; j++) {
            if (config->hunter_templates[i].sprites[j]) {
                free(config->hunter_templates[i].sprites[j]);
                config->hunter_templates[i].sprites[j] = NULL;
            }
        }
    }
    free(config->hunter_templates);
    config->hunter_templates = NULL;
}
