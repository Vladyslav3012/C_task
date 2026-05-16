/* Консольна версія меню (запасний варіант без GTK) */

#include "programs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

static void run_blocking(const Program *prog)
{
    pid_t pid;
    int status;
    char err[256];

    if (!programs_executable(prog->path)) {
        printf("  Помилка: «%s» недоступний.\n", prog->path);
        return;
    }

    pid = fork();
    if (pid < 0) {
        perror("fork");
        return;
    }
    if (pid == 0) {
        programs_chdir_home();
        if (programs_launch(prog, err, sizeof err) < 0) {
            fprintf(stderr, "%s\n", err);
            _exit(127);
        }
    }
    waitpid(pid, &status, 0);
    if (WIFEXITED(status))
        printf("  Код завершення: %d\n", WEXITSTATUS(status));
}

int main(void)
{
    char buf[64];
    int choice;

    programs_chdir_home();
    if (programs_load(CONF_FILE) < 0)
        fprintf(stderr, "Створіть %s\n", CONF_FILE);

    for (;;) {
        printf("\n=== Меню (консоль) ===\n");
        for (int i = 0; i < programs_count(); i++) {
            const Program *p = programs_get(i);
            printf("  %d. %s — %s\n", i + 1, p->name, p->desc);
        }
        printf("  r — оновити, 0 — вихід\n> ");
        if (!fgets(buf, sizeof buf, stdin))
            break;
        if (buf[0] == 'r' || buf[0] == 'R') {
            programs_load(CONF_FILE);
            continue;
        }
        choice = atoi(buf);
        if (choice == 0)
            break;
        if (choice >= 1 && choice <= programs_count())
            run_blocking(programs_get(choice - 1));
    }
    return 0;
}
