/*
 * GOLD_code_correlation_analysis — головне меню (GTK3).
 */

#include "gui_layout.h"
#include "gui_handlers.h"
#include "ui_params.h"
#include "indication.h"
#include "gold_config.h"
#include "programs.h"

#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    GoldApp *app = gold_app_get();

    (void)argv;
    gtk_init(&argc, &argv);
    gui_apply_css();

    memset(app, 0, sizeof *app);
    programs_chdir_home();
    gold_config_load(GOLD_CONF_FILE, &app->config);

    gui_layout_build(app);
    ui_params_fill_combos(app);
    indication_set_defaults(app);

    gui_handlers_connect(app);
    g_signal_connect(app->window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(app->window);
    gtk_main();
    return 0;
}
