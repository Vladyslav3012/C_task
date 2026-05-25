#ifndef GOLD_CONFIG_H
#define GOLD_CONFIG_H

#include <stddef.h>

#define GOLD_CONF_FILE "config/gold.conf"
#define GOLD_COMPUTE_DEFAULT "./bin/gold_compute"
#define GOLD_VIEWER_DEFAULT  "./bin/gold_viewer"
#define PATH_MAX_GOLD 512

typedef struct {
    char compute_path[PATH_MAX_GOLD];
    char viewer_path[PATH_MAX_GOLD];
} GoldConfig;

int gold_config_load(const char *filename, GoldConfig *cfg);
int gold_config_save(const char *filename, const GoldConfig *cfg);
void gold_config_defaults(GoldConfig *cfg);

#endif
