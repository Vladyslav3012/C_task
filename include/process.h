#ifndef PROCESS_H
#define PROCESS_H

#include "programs.h"

typedef struct {
#ifdef _WIN32
    void *process; /* HANDLE */
    unsigned long pid;
#else
    int pid;
#endif
    int active;
} ChildProc;

int programs_spawn_async(const Program *prog, char *errbuf, size_t errlen);

int programs_spawn_path_argv(const char *path, char *const argv[],
                             ChildProc *child, char *errbuf, size_t errlen);

void programs_child_kill(ChildProc *child);
int programs_child_poll(ChildProc *child, int *exit_code);

int programs_run_blocking(const Program *prog, char *errbuf, size_t errlen, int *exit_code);

int programs_resolve_path(const char *path, char *buf, size_t buflen);

#endif
