#include "gold_config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static void trim(char *s)
{
    size_t i = 0, j, n;
    if (!s)
        return;
    n = strlen(s);
    while (i < n && (s[i] == ' ' || s[i] == '\t' || s[i] == '\r'))
        i++;
    j = n;
    while (j > i && (s[j - 1] == ' ' || s[j - 1] == '\t' ||
                     s[j - 1] == '\r' || s[j - 1] == '\n'))
        j--;
    if (i > 0)
        memmove(s, s + i, j - i);
    s[j - i] = '\0';
}

void gold_config_defaults(GoldConfig *cfg)
{
    if (!cfg)
        return;
    strncpy(cfg->compute_path, GOLD_COMPUTE_DEFAULT, PATH_MAX_GOLD - 1);
    cfg->compute_path[PATH_MAX_GOLD - 1] = '\0';
    strncpy(cfg->viewer_path, GOLD_VIEWER_DEFAULT, PATH_MAX_GOLD - 1);
    cfg->viewer_path[PATH_MAX_GOLD - 1] = '\0';
}

int gold_config_load(const char *filename, GoldConfig *cfg)
{
    FILE *f;
    char line[256];
    char key[64], val[PATH_MAX_GOLD];

    if (!cfg)
        return -1;
    gold_config_defaults(cfg);

    f = fopen(filename, "r");
    if (!f)
        return -1;

    while (fgets(line, sizeof line, f)) {
        trim(line);
        if (line[0] == '\0' || line[0] == '#')
            continue;
        if (sscanf(line, " %63[^=] = %511[^\n]", key, val) < 2 &&
            sscanf(line, " %63[^=]=%511[^\n]", key, val) < 2)
            continue;
        trim(key);
        trim(val);
        if (strcmp(key, "compute") == 0)
            strncpy(cfg->compute_path, val, PATH_MAX_GOLD - 1);
        else if (strcmp(key, "viewer") == 0)
            strncpy(cfg->viewer_path, val, PATH_MAX_GOLD - 1);
        cfg->compute_path[PATH_MAX_GOLD - 1] = '\0';
        cfg->viewer_path[PATH_MAX_GOLD - 1] = '\0';
    }
    fclose(f);
    return 0;
}

int gold_config_save(const char *filename, const GoldConfig *cfg)
{
    FILE *f;

    if (!cfg)
        return -1;
    f = fopen(filename, "w");
    if (!f)
        return -1;
    fprintf(f, "# Шляхи відносно LAUNCHER_HOME\n");
    fprintf(f, "compute=%s\n", cfg->compute_path);
    fprintf(f, "viewer=%s\n", cfg->viewer_path);
    fclose(f);
    return 0;
}
