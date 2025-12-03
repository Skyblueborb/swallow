#ifndef CONF_H
#define CONF_H

#include "types.h"

conf_t read_config(const char* filename);
void free_config(conf_t* config);

#endif  // CONF_H
