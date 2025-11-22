#include "conf.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"

static const ConfigMapEntry global_key_map[] = {
        {"window_height", offsetof(conf_t, window_height), TYPE_INT},
        {"window_width", offsetof(conf_t, window_width), TYPE_INT},
        {"star_quota", offsetof(conf_t, star_quota), TYPE_INT},
        {"timer", offsetof(conf_t, timer), TYPE_FLOAT},
        {"star_spawn", offsetof(conf_t, star_spawn), TYPE_FLOAT},
        {"hunter_spawn", offsetof(conf_t, hunter_spawn), TYPE_FLOAT},
        {"min_speed", offsetof(conf_t, min_speed), TYPE_INT},
        {"max_speed", offsetof(conf_t, max_speed), TYPE_INT},
};

static const ConfigMapEntry hunter_key_map[] = {
        {"width", offsetof(HunterTypes, width), TYPE_INT},
        {"height", offsetof(HunterTypes, height), TYPE_INT},
        {"bounces", offsetof(HunterTypes, bounces), TYPE_INT},
        {"speed", offsetof(HunterTypes, speed), TYPE_INT},
};

static const int num_global_keys = sizeof(global_key_map) / sizeof(global_key_map[0]);
static const int num_hunter_keys = sizeof(hunter_key_map) / sizeof(hunter_key_map[0]);

static void strip_newline(char* str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
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

static void parse_values(conf_t* config, int hunter_idx, const char* key, const char* value) {
    void* base_ptr;
    const ConfigMapEntry* map;
    int count;

    if (hunter_idx < 0) {
        base_ptr = config;
        map = global_key_map;
        count = num_global_keys;
    } else {
        if (strncmp(key, "sprite", 6) == 0) {
            parse_sprite(config, hunter_idx, key, value);
            return;
        }
        base_ptr = &config->hunter_templates[hunter_idx];
        map = hunter_key_map;
        count = num_hunter_keys;
    }

    for (int i = 0; i < count; i++) {
        if (strcmp(key, map[i].key_name) == 0) {
            void* target = (char*)base_ptr + map[i].offset;

            if (map[i].type == TYPE_INT) {
                *(int*)target = atoi(value);
            } else if (map[i].type == TYPE_FLOAT) {
                *(float*)target = atof(value);
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
