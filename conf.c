#include "conf.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "utils.h"

static const ColorNameEntry* get_color_map(int* count) {
    static const ColorNameEntry map[] = {
        {"default",     PAIR_DEFAULT},
        {"player",      PAIR_PLAYER},
        {"star",        PAIR_STAR},
        {"red_1",       C_RED_1},
        {"red_2",       C_RED_2},
        {"red_3",       C_RED_3},
        {"red_4",       C_RED_4},
        {"red_5",       C_RED_5},
        {"green_1",     C_GREEN_1},
        {"green_2",     C_GREEN_2},
        {"green_3",     C_GREEN_3},
        {"green_4",     C_GREEN_4},
        {"green_5",     C_GREEN_5},
        {"blue_1",      C_BLUE_1},
        {"blue_2",      C_BLUE_2},
        {"blue_3",      C_BLUE_3},
        {"blue_4",      C_BLUE_4},
        {"blue_5",      C_BLUE_5},
        {"yellow_1",    C_YELLOW_1},
        {"yellow_2",    C_YELLOW_2},
        {"yellow_3",    C_YELLOW_3},
        {"yellow_4",    C_YELLOW_4},
        {"yellow_5",    C_YELLOW_5},
        {"purple_1",    C_PURPLE_1},
        {"purple_2",    C_PURPLE_2},
        {"purple_3",    C_PURPLE_3},
        {"purple_4",    C_PURPLE_4},
        {"purple_5",    C_PURPLE_5},
        {"cyan_1",      C_CYAN_1},
        {"cyan_2",      C_CYAN_2},
        {"cyan_3",      C_CYAN_3},
        {"cyan_4",      C_CYAN_4},
        {"cyan_5",      C_CYAN_5},
        {"grey_1",      C_GREY_1},
        {"grey_2",      C_GREY_2},
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
    };
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
        {"color",   offsetof(HunterTypes, color), TYPE_COLOR},
    };
    *count = sizeof(map) / sizeof(map[0]);
    return map;
}

static void parse_sprite(conf_t* config, int hunter_idx, const char* key, const char* value) {
    HunterTypes* hunter = &config->hunter_templates[hunter_idx];
    size_t size = hunter->width * hunter->height + 1;
    if (hunter->sprites[0] == NULL) {
        for (int i = 0; i < NUM_DIRECTIONS; i++) {
            hunter->sprites[i] = malloc(size);
        }
    }

    switch (key[7]) {
        case 'u':
            strncpy(hunter->sprites[DIR_UP], value, size);
            break;
        case 'd':
            strncpy(hunter->sprites[DIR_DOWN], value, size);
            break;
        case 'l':
            strncpy(hunter->sprites[DIR_LEFT], value, size);
            break;
        case 'r':
            strncpy(hunter->sprites[DIR_RIGHT], value, size);
            break;
    }
}

static int parse_color_name(const char* value) {
    int count;
    const ColorNameEntry* map = get_color_map(&count);

    for (int i = 0; i < count; i++) {
        if (strcmp(value, map[i].name) == 0) {
            return (int)map[i].value;
        }
    }
    return (int)PAIR_DEFAULT;
}

static void parse_values(conf_t* config, int hunter_idx, const char* key, const char* value) {
    void* base_ptr;
    const ConfigMapEntry* map;
    int count;

    if (hunter_idx < 0) {
        base_ptr = config;
        map = get_global_key_map(&count);
    } else {
        if (strncmp(key, "sprite", 6) == 0) {
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
                *(float*)target = atof(value);
            } else if (map[i].type == TYPE_COLOR) {
                *(int*)target = parse_color_name(value);
            }
        }
    }
}

static void process_config_line(const char* line, conf_t* config, int* hunter_idx) {
    char key[100] = {0}, value[100] = {0};
    int items = sscanf(line, "%99s %99s", key, value);

    if (items <= 0) return;

    if (strcmp(key, "hunter_template") == 0) {
        (*hunter_idx)++;
        if (*hunter_idx >= 5) *hunter_idx = 4;
    } else {
        parse_values(config, *hunter_idx, key, value);
    }
}

conf_t read_config(char* filename) {
    conf_t config = {0};
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Error opening config file");
        return config;
    }

    char line[256] = {0};
    int hunter_index = -1;

    while (fgets(line, sizeof(line), file)) {
        strip_newline(line);
        if (line[0] == '#' || line[0] == '\0') continue;

        process_config_line(line, &config, &hunter_index);
    }
    fclose(file);
    return config;
}

void free_config(conf_t* config) {
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < NUM_DIRECTIONS; j++) {
            if (config->hunter_templates[i].sprites[j]) {
                free(config->hunter_templates[i].sprites[j]);
            }
        }
    }
}
