/* Консольна версія меню (запасний варіант без GTK) */

#include "programs.h"
#include "process.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
        if (choice >= 1 && choice <= programs_count()) {
            const Program *p = programs_get(choice - 1);
            char err[256];
            int code = -1;

            if (programs_run_blocking(p, err, sizeof err, &code) < 0)
                printf("  Помилка: %s\n", err);
            else if (code >= 0)
                printf("  Код завершення: %d\n", code);
        }
    }
    return 0;
}
