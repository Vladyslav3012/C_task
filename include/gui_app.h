#ifndef GUI_APP_H
#define GUI_APP_H

#include <gtk/gtk.h>
#include "gold_config.h"
#include "process.h"

typedef struct GoldApp GoldApp;

struct GoldApp {
    GtkWidget *window;
    GtkWidget *progress;
    GtkWidget *combo_n;
    GtkWidget *combo_corr;
    GtkWidget *combo_crit;
    GtkWidget *combo_k;
    GtkWidget *entry_prim_b;
    GtkWidget *entry_prim_o;
    GtkWidget *entry_pairs_b;
    GtkWidget *entry_pairs_o;
    GtkWidget *entry_ens1;
    GtkWidget *entry_ens2;
    GtkWidget *entry_ens3;
    GtkWidget *btn_start;
    GtkWidget *btn_finish;
    GtkWidget *btn_reset;
    GtkWidget *btn_print;
    GtkWidget *btn_options;
    GtkWidget *btn_result[8];
    GoldConfig config;
    ChildProc compute_child;
    guint progress_timer_id;
    int computing;
    int results_ready;
};

GoldApp *gold_app_get(void);

#endif
