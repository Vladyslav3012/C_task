#include "programs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define LINE_LEN 1024

static Program programs[MAX_PROGRAMS];
static int program_count = 0;

static void trim_inplace(char *s)
{
    size_t i, j, n;
    if (!s)
        return;
    n = strlen(s);
    i = 0;
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

int programs_chdir_home(void)
{
    const char *home = getenv("LAUNCHER_HOME");
    if (!home || home[0] == '\0')
        return 0;
    if (chdir(home) != 0) {
        fprintf(stderr, "LAUNCHER_HOME=%s: %s\n", home, strerror(errno));
        return -1;
    }
    return 0;
}

int programs_load(const char *filename)
{
    FILE *f;
    char line[LINE_LEN];
    char *p1, *p2, *p3;

    program_count = 0;
    f = fopen(filename, "r");
    if (!f)
        return -1;

    while (fgets(line, sizeof line, f) && program_count < MAX_PROGRAMS) {
        trim_inplace(line);
        if (line[0] == '\0' || line[0] == '#')
            continue;

        p1 = line;
        p2 = strchr(p1, '|');
        if (!p2)
            continue;
        *p2++ = '\0';
        p3 = strchr(p2, '|');
        if (!p3)
            continue;
        *p3++ = '\0';

        trim_inplace(p1);
        trim_inplace(p2);
        trim_inplace(p3);
        if (p1[0] == '\0' || p2[0] == '\0')
            continue;

        strncpy(programs[program_count].name, p1, NAME_LEN - 1);
        programs[program_count].name[NAME_LEN - 1] = '\0';
        strncpy(programs[program_count].path, p2, PATH_LEN - 1);
        programs[program_count].path[PATH_LEN - 1] = '\0';
        strncpy(programs[program_count].desc, p3, DESC_LEN - 1);
        programs[program_count].desc[DESC_LEN - 1] = '\0';
        program_count++;
    }

    fclose(f);
    return program_count;
}

int programs_count(void)
{
    return program_count;
}

const Program *programs_get(int index)
{
    if (index < 0 || index >= program_count)
        return NULL;
    return &programs[index];
}

int programs_executable(const char *path)
{
    return access(path, X_OK) == 0;
}

int programs_launch(const Program *prog, char *errbuf, size_t errlen)
{
    const char *home;
    char *argv[2];

    if (!prog) {
        if (errbuf && errlen)
            snprintf(errbuf, errlen, "Програму не обрано.");
        return -1;
    }
    if (!programs_executable(prog->path)) {
        if (errbuf && errlen)
            snprintf(errbuf, errlen,
                     "Файл «%s» недоступний.\nЗберіть програму (make) або перевірте config/programs.conf.",
                     prog->path);
        return -1;
    }

    home = getenv("LAUNCHER_HOME");
    if (home && home[0] && chdir(home) != 0) {
        if (errbuf && errlen)
            snprintf(errbuf, errlen, "chdir(%s): %s", home, strerror(errno));
        return -1;
    }

    argv[0] = (char *)prog->path;
    argv[1] = NULL;

    if (execv(prog->path, argv) < 0) {
        if (errbuf && errlen)
            snprintf(errbuf, errlen, "exec: %s", strerror(errno));
        return -1;
    }
    return 0;
}
