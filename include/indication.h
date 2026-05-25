#ifndef INDICATION_H
#define INDICATION_H

#include "gui_app.h"

#define SUMMARY_FILE "output/summary.json"

void indication_set_defaults(GoldApp *app);
void indication_load_summary(GoldApp *app);
void indication_clear_output_files(void);
int indication_result_file_exists(const char *basename);
void indication_update_result_buttons(GoldApp *app);

#endif
