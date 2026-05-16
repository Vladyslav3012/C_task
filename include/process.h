#ifndef PROCESS_H
#define PROCESS_H

#include "programs.h"

/* Запуск у фоні (GUI): Linux fork, Windows CreateProcess */
int programs_spawn_async(const Program *prog, char *errbuf, size_t errlen);

/* Запуск з очікуванням (CLI) */
int programs_run_blocking(const Program *prog, char *errbuf, size_t errlen, int *exit_code);

/* Шлях у buf, якщо файл існує (на Win додає .exe) */
int programs_resolve_path(const char *path, char *buf, size_t buflen);

#endif
