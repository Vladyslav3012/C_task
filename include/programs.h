#ifndef PROGRAMS_H
#define PROGRAMS_H

#include <stddef.h>

#define MAX_PROGRAMS 64
#define NAME_LEN 128
#define PATH_LEN 512
#define DESC_LEN 256
#define CONF_FILE "config/programs.conf"
#define OUTPUT_DIR "output"

typedef struct {
    char name[NAME_LEN];
    char path[PATH_LEN];
    char desc[DESC_LEN];
} Program;

int programs_chdir_home(void);
int programs_load(const char *filename);
int programs_count(void);
const Program *programs_get(int index);
int programs_executable(const char *path);
int programs_launch(const Program *prog, char *errbuf, size_t errlen);

#endif
