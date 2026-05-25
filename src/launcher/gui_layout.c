#include "gui_layout.h"

#include <string.h>

static GtkWidget *make_entry_readonly(const char *text)
{
    GtkWidget *e = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(e), text);
    gtk_editable_set_editable(GTK_EDITABLE(e), FALSE);
    gtk_widget_set_size_request(e, 56, -1);
    return e;
}

static GtkWidget *indication_group(const char *title, GtkWidget *e1, GtkWidget *e2, GtkWidget *e3)
{
    GtkWidget *v = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    GtkWidget *lbl = gtk_label_new(title);
    GtkWidget *h = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_label_set_xalign(GTK_LABEL(lbl), 0.0);
    gtk_label_set_line_wrap(GTK_LABEL(lbl), TRUE);
    gtk_label_set_max_width_chars(GTK_LABEL(lbl), 28);
    gtk_box_pack_start(GTK_BOX(v), lbl, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(h), e1, FALSE, FALSE, 0);
    if (e2)
        gtk_box_pack_start(GTK_BOX(h), e2, FALSE, FALSE, 0);
    if (e3)
        gtk_box_pack_start(GTK_BOX(h), e3, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(v), h, FALSE, FALSE, 0);
    return v;
}

static GtkWidget *param_row(const char *label, GtkWidget *combo)
{
    GtkWidget *h = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    GtkWidget *lbl = gtk_label_new(label);
    gtk_label_set_xalign(GTK_LABEL(lbl), 0.0);
    gtk_label_set_line_wrap(GTK_LABEL(lbl), TRUE);
    gtk_label_set_max_width_chars(GTK_LABEL(lbl), 42);
    gtk_widget_set_hexpand(lbl, TRUE);
    gtk_widget_set_size_request(combo, 120, -1);
    gtk_box_pack_start(GTK_BOX(h), lbl, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(h), combo, FALSE, FALSE, 0);
    return h;
}

GtkWidget *gui_layout_build(GoldApp *app)
{
    GtkWidget *outer, *border, *vbox, *top, *params, *params_l, *params_r;
    GtkWidget *mid, *ind_row, *bot, *results_box, *lbl_res;
    GtkWidget *btn_manual, *btn_info, *header;
    GtkWidget *combo_n, *combo_corr, *combo_crit, *combo_k;
    GtkWidget *btn_start, *btn_finish, *btn_reset, *btn_print, *btn_opts;
    const char *result_labels[] = {
        "АКФ_1", "АКФ_2", "ВКФ_1", "ВКФ_2",
        "SRT_1", "SRT_2", "USRT_1", "USRT_2"
    };
    int i;

    app->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(app->window),
        "Програма [GOLD_code_correlation_analysis], версія 2, C language");
    gtk_window_set_default_size(GTK_WINDOW(app->window), 960, 640);
    gtk_window_set_position(GTK_WINDOW(app->window), GTK_WIN_POS_CENTER);

    outer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_name(outer, "gold-outer");
    gtk_container_add(GTK_CONTAINER(app->window), outer);

    border = gtk_frame_new(NULL);
    gtk_widget_set_name(border, "gold-border");
    gtk_frame_set_shadow_type(GTK_FRAME(border), GTK_SHADOW_ETCHED_IN);
    gtk_box_pack_start(GTK_BOX(outer), border, TRUE, TRUE, 8);

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);
    gtk_container_add(GTK_CONTAINER(border), vbox);

    /* Верхній ряд */
    top = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    btn_manual = gtk_button_new_with_label("USER MANUAL");
    gtk_widget_set_name(btn_manual, "btn-manual");
    header = gtk_label_new(
        "Програма [GOLD_code_correlation_analysis], версія 2, C language");
    gtk_widget_set_hexpand(header, TRUE);
    gtk_label_set_xalign(GTK_LABEL(header), 0.5);
    gtk_widget_set_name(header, "gold-title");
    btn_info = gtk_button_new_with_label("INFO (осн. парам.)");
    gtk_widget_set_name(btn_info, "btn-info");
    gtk_box_pack_start(GTK_BOX(top), btn_manual, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(top), header, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(top), btn_info, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), top, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(vbox), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL),
                       FALSE, FALSE, 4);

    /* Параметри 2x2 */
    params = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    params_l = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    params_r = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);

    combo_n = gtk_combo_box_text_new();
    combo_corr = gtk_combo_box_text_new();
    combo_crit = gtk_combo_box_text_new();
    combo_k = gtk_combo_box_text_new();
    app->combo_n = combo_n;
    app->combo_corr = combo_corr;
    app->combo_crit = combo_crit;
    app->combo_k = combo_k;

    gtk_box_pack_start(GTK_BOX(params_l),
        param_row("Вибрати степінь (n) базових [$k_b(X)$] примітивних (prim) поліномів",
                  combo_n), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(params_l),
        param_row("Вибрати тип кореляції [для критеріїв та індикації (АКФ або ВКФ)]",
                  combo_corr), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(params_r),
        param_row("Вибрати критерій впорядкування кодів по усереднених паразитних кореляціях",
                  combo_crit), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(params_r),
        param_row("Вибрати потрібну кількість (K) кодів Голда з мінімальними кореляціями",
                  combo_k), FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(params), params_l, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(params), params_r, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), params, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(vbox), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL),
                       FALSE, FALSE, 4);

    /* START | progress | FINISH */
    mid = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    btn_start = gtk_button_new_with_label("START");
    gtk_widget_set_name(btn_start, "btn-start");
    gtk_widget_set_size_request(btn_start, 100, 36);
    app->progress = gtk_progress_bar_new();
    gtk_widget_set_hexpand(app->progress, TRUE);
    gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(app->progress), TRUE);
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(app->progress), 0.0);
    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(app->progress), "0%");
    btn_finish = gtk_button_new_with_label("FINISH");
    gtk_widget_set_name(btn_finish, "btn-finish");
    gtk_widget_set_size_request(btn_finish, 100, 36);
    app->btn_start = btn_start;
    app->btn_finish = btn_finish;
    gtk_box_pack_start(GTK_BOX(mid), btn_start, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(mid), app->progress, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(mid), btn_finish, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), mid, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(vbox), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL),
                       FALSE, FALSE, 4);

    /* Індикація */
    app->entry_prim_b = make_entry_readonly("10_Б");
    app->entry_prim_o = make_entry_readonly("2_Б");
    app->entry_pairs_b = make_entry_readonly("10_пар");
    app->entry_pairs_o = make_entry_readonly("2_пар");
    app->entry_ens1 = make_entry_readonly("16");
    app->entry_ens2 = make_entry_readonly("2");
    app->entry_ens3 = make_entry_readonly("+/-");

    ind_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_pack_start(GTK_BOX(ind_row),
        indication_group("Індикація всіх (prim) поліномів $|k(X)|$ степеня (n)",
                         app->entry_prim_b, app->entry_prim_o, NULL),
        TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(ind_row),
        indication_group("Індикація оптимальних пар поліномів та індексів децимації "
                         "[$k_{опт}(X), k_{спр}(X), d$]",
                         app->entry_pairs_b, app->entry_pairs_o, NULL),
        TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(ind_row),
        indication_group("Індикація всіх (Q) ансамблів кодів Голда для заданого степеня поліномів (n)",
                         app->entry_ens1, app->entry_ens2, app->entry_ens3),
        TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), ind_row, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(vbox), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL),
                       FALSE, FALSE, 4);

    /* Низ: PRINT Опції RESET | результати */
    bot = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    btn_print = gtk_button_new_with_label("PRINT");
    btn_opts = gtk_button_new_with_label("Опції");
    btn_reset = gtk_button_new_with_label("RESET");
    gtk_widget_set_name(btn_print, "btn-print");
    gtk_widget_set_name(btn_opts, "btn-options");
    gtk_widget_set_name(btn_reset, "btn-reset");
    app->btn_print = btn_print;
    app->btn_options = btn_opts;
    app->btn_reset = btn_reset;

    results_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    lbl_res = gtk_label_new("Індикація вихідних результатів");
    gtk_label_set_xalign(GTK_LABEL(lbl_res), 0.5);
    {
        GtkWidget *row1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
        GtkWidget *row2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
        for (i = 0; i < 4; i++) {
            app->btn_result[i] = gtk_button_new_with_label(result_labels[i]);
            gtk_widget_set_name(app->btn_result[i], "btn-result");
            gtk_box_pack_start(GTK_BOX(row1), app->btn_result[i], TRUE, TRUE, 0);
        }
        for (i = 4; i < 8; i++) {
            app->btn_result[i] = gtk_button_new_with_label(result_labels[i]);
            gtk_widget_set_name(app->btn_result[i], "btn-result");
            gtk_box_pack_start(GTK_BOX(row2), app->btn_result[i], TRUE, TRUE, 0);
        }
        gtk_box_pack_start(GTK_BOX(results_box), lbl_res, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(results_box), row1, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(results_box), row2, FALSE, FALSE, 0);
    }

    {
        GtkWidget *left_btns = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
        gtk_box_pack_start(GTK_BOX(left_btns), btn_print, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(left_btns), btn_opts, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(left_btns), btn_reset, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(bot), left_btns, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(bot), results_box, TRUE, TRUE, 0);
    }
    gtk_box_pack_start(GTK_BOX(vbox), bot, FALSE, FALSE, 0);

    g_object_set_data(G_OBJECT(app->window), "btn_manual", btn_manual);
    g_object_set_data(G_OBJECT(app->window), "btn_info", btn_info);

    return app->window;
}

void gui_apply_css(void)
{
    const char *css =
        "window { background-color: #ffffff; }"
        "#gold-border { border: 3px double #cc0000; background-color: #ffffff; "
        "               padding: 4px; }"
        "#gold-title { color: #1a5276; font-weight: bold; font-size: 11pt; }"
        "button.btn-manual, button.btn-info { "
        "  background-image: none; background-color: #c0392b; color: #ffffff; "
        "  font-weight: bold; padding: 6px 10px; }"
        "#btn-start { background-image: none; background-color: #27ae60; "
        "  color: #ffffff; font-weight: bold; }"
        "#btn-finish { background-image: none; background-color: #ffffff; "
        "  color: #27ae60; font-weight: bold; border: 2px solid #27ae60; }"
        "#btn-reset { background-image: none; background-color: #e74c3c; "
        "  color: #ffffff; font-weight: bold; }"
        "#btn-print, #btn-options { background-image: none; background-color: #8e44ad; "
        "  color: #ffffff; font-weight: bold; }"
        "#btn-result { background-image: none; background-color: #ecf0f1; "
        "  color: #2c3e50; font-weight: bold; min-width: 52px; }"
        "#btn-result:disabled { color: #bdc3c7; }"
        "label { color: #1a252f; }"
        "entry { background-color: #ffffff; }";

    GtkCssProvider *p = gtk_css_provider_new();
    gtk_css_provider_load_from_data(p, css, -1, NULL);
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(p),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(p);
}
