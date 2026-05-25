#ifndef UI_PARAMS_H
#define UI_PARAMS_H

#include "gui_app.h"

typedef struct {
    int n;
    int k;
    int corr_index;  /* 0 periodic, 1 aperiodic */
    int crit_index;  /* 0..3 */
} UiParams;

void ui_params_fill_combos(GoldApp *app);
void ui_params_reset_defaults(GoldApp *app);
void ui_params_read(GoldApp *app, UiParams *out);
void ui_params_build_argv(const UiParams *p, char *argv[], int *argc);
const char *ui_params_corr_label(int index);
const char *ui_params_crit_label(int index);

#endif
