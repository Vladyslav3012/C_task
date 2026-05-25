#include "ui_params.h"

#include <stdio.h>
#include <string.h>

static const char *CORR_LABELS[] = { "Періодична", "Аперіодична", NULL };
static const char *CRIT_LABELS[] = { "СРЗН + СКВ", "СРЗН", "Макс", "Мін", NULL };
static const char *CORR_ARG[] = { "periodic", "aperiodic", NULL };
static const char *CRIT_ARG[] = { "srzn_skv", "srzn", "max", "min", NULL };

const char *ui_params_corr_label(int index)
{
    if (index < 0 || index > 1)
        return CORR_LABELS[0];
    return CORR_LABELS[index];
}

const char *ui_params_crit_label(int index)
{
    if (index < 0 || index > 3)
        return CRIT_LABELS[0];
    return CRIT_LABELS[index];
}

static void combo_fill(GtkComboBoxText *combo, const char **labels, int select)
{
    int i;
    gtk_combo_box_text_remove_all(combo);
    for (i = 0; labels[i]; i++)
        gtk_combo_box_text_append_text(combo, labels[i]);
    if (select >= 0 && select < i)
        gtk_combo_box_set_active(GTK_COMBO_BOX(combo), select);
}

void ui_params_fill_combos(GoldApp *app)
{
    char buf[16];
    int i;

    gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(app->combo_n));
    for (i = 3; i <= 12; i++) {
        snprintf(buf, sizeof buf, "%d", i);
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->combo_n), buf);
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(app->combo_n), 2); /* n=5 */

    combo_fill(GTK_COMBO_BOX_TEXT(app->combo_corr), CORR_LABELS, 0);
    combo_fill(GTK_COMBO_BOX_TEXT(app->combo_crit), CRIT_LABELS, 0);

    gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(app->combo_k));
    for (i = 1; i <= 63; i++) {
        snprintf(buf, sizeof buf, "%d", i);
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->combo_k), buf);
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(app->combo_k), 32); /* K=33 */
}

void ui_params_reset_defaults(GoldApp *app)
{
    ui_params_fill_combos(app);
}

void ui_params_read(GoldApp *app, UiParams *out)
{
    const char *t;
    if (!out)
        return;
    t = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(app->combo_n));
    out->n = t ? atoi(t) : 5;
    if (t)
        g_free((gpointer)t);
    t = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(app->combo_k));
    out->k = t ? atoi(t) : 33;
    if (t)
        g_free((gpointer)t);
    out->corr_index = gtk_combo_box_get_active(GTK_COMBO_BOX(app->combo_corr));
    out->crit_index = gtk_combo_box_get_active(GTK_COMBO_BOX(app->combo_crit));
    if (out->corr_index < 0)
        out->corr_index = 0;
    if (out->crit_index < 0)
        out->crit_index = 0;
}

void ui_params_build_argv(const UiParams *p, char *argv[], int *argc)
{
    static char nbuf[16], kbuf[16];
    int i = 0;

    argv[i++] = (char *)"--n";
    snprintf(nbuf, sizeof nbuf, "%d", p->n);
    argv[i++] = nbuf;
    argv[i++] = (char *)"--corr";
    argv[i++] = (char *)CORR_ARG[p->corr_index];
    argv[i++] = (char *)"--crit";
    argv[i++] = (char *)CRIT_ARG[p->crit_index];
    argv[i++] = (char *)"--k";
    snprintf(kbuf, sizeof kbuf, "%d", p->k);
    argv[i++] = kbuf;
    argv[i++] = NULL;
    if (argc)
        *argc = i - 1;
}
