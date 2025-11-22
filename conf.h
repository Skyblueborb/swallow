#ifndef CONF_H
#define CONF_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "types.h"

void strip_newline(char *str);
conf_t read_config(char *filename);
void process_config_line(const char *line, conf_t *config, int *hunter_idx);
void parse_values(conf_t *config, int hunter_idx, const char *key, const char *value);
void parse_sprite(conf_t *config, int hunter_idx, const char *key, const char *value);
void free_config(conf_t *config);

#endif // CONF_H
