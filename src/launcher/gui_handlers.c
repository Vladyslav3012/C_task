#include "gui_handlers.h"

#include "indication.h"
#include "ui_params.h"
#include "programs.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

static GoldApp g_app;

GoldApp *gold_app_get(void)
{
    return &g_app;
}

static void set_progress(GoldApp *app, double frac)
{
    char t[16];
    if (frac < 0.0)
        frac = 0.0;
    if (frac > 1.0)
        frac = 1.0;
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(app->progress), frac);
    snprintf(t, sizeof t, "%d%%", (int)(frac * 100.0 + 0.5));
    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(app->progress), t);
}

static void show_info(GtkWindow *parent, const char *title, const char *msg)
{
    GtkWidget *d = gtk_message_dialog_new(
        parent, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", msg);
    gtk_window_set_title(GTK_WINDOW(d), title);
    gtk_dialog_run(GTK_DIALOG(d));
    gtk_widget_destroy(d);
}

static void show_error(GtkWindow *parent, const char *title, const char *msg)
{
    GtkWidget *d = gtk_message_dialog_new(
        parent, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "%s", msg);
    gtk_window_set_title(GTK_WINDOW(d), title);
    gtk_dialog_run(GTK_DIALOG(d));
    gtk_widget_destroy(d);
}

static void set_busy(GoldApp *app, gboolean busy)
{
    app->computing = busy ? 1 : 0;
    gtk_widget_set_sensitive(app->combo_n, !busy);
    gtk_widget_set_sensitive(app->combo_corr, !busy);
    gtk_widget_set_sensitive(app->combo_crit, !busy);
    gtk_widget_set_sensitive(app->combo_k, !busy);
    gtk_widget_set_sensitive(app->btn_start, !busy);
}

static gboolean progress_tick(gpointer data)
{
    GoldApp *app = data;
    double f;
    int exit_code = -1;

    if (!app->computing)
        return G_SOURCE_REMOVE;

    f = gtk_progress_bar_get_fraction(GTK_PROGRESS_BAR(app->progress));
    if (f < 0.9)
        set_progress(app, f + 0.02);

    if (programs_child_poll(&app->compute_child, &exit_code)) {
        app->progress_timer_id = 0;
        set_busy(app, FALSE);
        if (exit_code == 0) {
            indication_load_summary(app);
            set_progress(app, 1.0);
            show_info(GTK_WINDOW(app->window), "START", "Обчислення завершено.");
        } else {
            set_progress(app, 0.0);
            show_error(GTK_WINDOW(app->window), "START",
                       "Обчислення завершилось з помилкою.");
        }
        return G_SOURCE_REMOVE;
    }
    return G_SOURCE_CONTINUE;
}

static void on_params_changed(GtkWidget *w, gpointer data)
{
    GoldApp *app = data;
    (void)w;
    if (app->computing)
        return;
    set_progress(app, 0.0);
}

static void on_manual(GtkWidget *w, gpointer data)
{
    GoldApp *app = data;
    GtkWidget *dlg, *scroll, *view;
    GtkTextBuffer *buf;
    FILE *f;
    char line[512];
    GString *text;

    (void)w;
    dlg = gtk_dialog_new_with_buttons("USER MANUAL",
        GTK_WINDOW(app->window),
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        "_Закрити", GTK_RESPONSE_CLOSE, NULL);
    gtk_window_set_default_size(GTK_WINDOW(dlg), 560, 420);

    scroll = gtk_scrolled_window_new(NULL, NULL);
    view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(view), GTK_WRAP_WORD);
    gtk_container_add(GTK_CONTAINER(scroll), view);

    buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
    text = g_string_new(NULL);
    f = fopen("docs/user_manual_uk.txt", "r");
    if (f) {
        while (fgets(line, sizeof line, f))
            g_string_append(text, line);
        fclose(f);
    } else {
        g_string_append(text,
            "Програма GOLD_code_correlation_analysis — меню аналізу кодів Голда.\n\n"
            "1. Оберіть параметри n, тип кореляції, критерій, K.\n"
            "2. Натисніть START для обчислення.\n"
            "3. Переглядайте результати кнопками АКФ/ВКФ/SRT.\n"
            "4. PRINT — звіт у output/report.txt.\n"
            "5. RESET — скинути параметри та індикацію.\n"
            "6. FINISH — вихід.\n");
    }
    gtk_text_buffer_set_text(buf, text->str, (gint)text->len);
    g_string_free(text, TRUE);

    gtk_container_set_border_width(GTK_CONTAINER(scroll), 8);
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dlg))),
                       scroll, TRUE, TRUE, 0);
    gtk_widget_show_all(dlg);
    gtk_dialog_run(GTK_DIALOG(dlg));
    gtk_widget_destroy(dlg);
}

static void on_info(GtkWidget *w, gpointer data)
{
    GoldApp *app = data;
    UiParams p;
    char msg[2048];
    const char *home = getenv("LAUNCHER_HOME");

    (void)w;
    ui_params_read(app, &p);
    snprintf(msg, sizeof msg,
             "Програма [GOLD_code_correlation_analysis], версія 2\n"
             "Мова: C (GTK3)\n\n"
             "Степінь n: %d\n"
             "Тип кореляції: %s\n"
             "Критерій: %s\n"
             "K кодів: %d\n\n"
             "compute: %s\n"
             "viewer: %s\n"
             "Каталог: %s\n"
             "Стан: %s",
             p.n, ui_params_corr_label(p.corr_index),
             ui_params_crit_label(p.crit_index), p.k,
             app->config.compute_path, app->config.viewer_path,
             home ? home : "(поточний)",
             app->computing ? "обчислення виконується" : "готово");
    show_info(GTK_WINDOW(app->window), "INFO (осн. парам.)", msg);
}

static void on_start(GtkWidget *w, gpointer data)
{
    GoldApp *app = data;
    UiParams p;
    char *argv[16];
    int argc;
    char err[512];

    (void)w;
    if (app->computing)
        return;

    if (!programs_executable(app->config.compute_path)) {
        show_error(GTK_WINDOW(app->window), "START",
                   "Обчислювальна програма не знайдена.\nЗберіть: make");
        return;
    }

    ui_params_read(app, &p);
    ui_params_build_argv(&p, argv, &argc);

    memset(&app->compute_child, 0, sizeof app->compute_child);
    if (programs_spawn_path_argv(app->config.compute_path, argv,
                                 &app->compute_child, err, sizeof err) < 0) {
        show_error(GTK_WINDOW(app->window), "START", err);
        return;
    }

    set_busy(app, TRUE);
    set_progress(app, 0.05);
    if (app->progress_timer_id)
        g_source_remove(app->progress_timer_id);
    app->progress_timer_id = g_timeout_add(50, progress_tick, app);
}

static void on_finish(GtkWidget *w, gpointer data)
{
    GoldApp *app = data;
    GtkWidget *dlg;
    int resp;

    (void)w;
    if (app->computing) {
        dlg = gtk_message_dialog_new(
            GTK_WINDOW(app->window), GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION,
            GTK_BUTTONS_YES_NO,
            "Обчислення виконується. Перервати та вийти?");
        resp = gtk_dialog_run(GTK_DIALOG(dlg));
        gtk_widget_destroy(dlg);
        if (resp != GTK_RESPONSE_YES)
            return;
        if (app->progress_timer_id) {
            g_source_remove(app->progress_timer_id);
            app->progress_timer_id = 0;
        }
        programs_child_kill(&app->compute_child);
        set_busy(app, FALSE);
        set_progress(app, 0.0);
    }
    gtk_main_quit();
}

static void on_reset(GtkWidget *w, gpointer data)
{
    GoldApp *app = data;
    (void)w;
    if (app->computing)
        return;
    ui_params_reset_defaults(app);
    indication_set_defaults(app);
    indication_clear_output_files();
    set_progress(app, 0.0);
}

static void on_print(GtkWidget *w, gpointer data)
{
    GoldApp *app = data;
    UiParams p;
    FILE *f;
    time_t now;
    char tim[64];
    static const char *rfiles[] = {
        "acf_1.txt", "acf_2.txt", "ccf_1.txt", "ccf_2.txt",
        "srt_1.txt", "srt_2.txt", "usrt_1.txt", "usrt_2.txt", NULL
    };
    int i;

    (void)w;
    ui_params_read(app, &p);
    f = fopen("output/report.txt", "w");
    if (!f) {
        show_error(GTK_WINDOW(app->window), "PRINT",
                   "Не вдалося створити output/report.txt");
        return;
    }
    now = time(NULL);
    strftime(tim, sizeof tim, "%Y-%m-%d %H:%M:%S", localtime(&now));
    fprintf(f, "Звіт GOLD_code_correlation_analysis\n");
    fprintf(f, "Дата: %s\n\n", tim);
    fprintf(f, "Параметри:\n  n = %d\n  кореляція = %s\n  критерій = %s\n  K = %d\n\n",
            p.n, ui_params_corr_label(p.corr_index),
            ui_params_crit_label(p.crit_index), p.k);
    fprintf(f, "Індикація:\n  prim: %s, %s\n  пари: %s, %s\n  ансамблі: %s, %s, %s\n\n",
            gtk_entry_get_text(GTK_ENTRY(app->entry_prim_b)),
            gtk_entry_get_text(GTK_ENTRY(app->entry_prim_o)),
            gtk_entry_get_text(GTK_ENTRY(app->entry_pairs_b)),
            gtk_entry_get_text(GTK_ENTRY(app->entry_pairs_o)),
            gtk_entry_get_text(GTK_ENTRY(app->entry_ens1)),
            gtk_entry_get_text(GTK_ENTRY(app->entry_ens2)),
            gtk_entry_get_text(GTK_ENTRY(app->entry_ens3)));
    fprintf(f, "Файли результатів:\n");
    for (i = 0; rfiles[i]; i++)
        fprintf(f, "  %s: %s\n", rfiles[i],
                indication_result_file_exists(rfiles[i]) ? "так" : "ні");
    fclose(f);
    show_info(GTK_WINDOW(app->window), "PRINT",
              "Збережено: output/report.txt");
}

static void on_options(GtkWidget *w, gpointer data)
{
    GoldApp *app = data;
    GtkWidget *dlg, *grid, *e_compute, *e_viewer;
    GtkWidget *lbl_c, *lbl_v;
    int resp;

    (void)w;
    dlg = gtk_dialog_new_with_buttons("Опції",
        GTK_WINDOW(app->window),
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        "_Скасувати", GTK_RESPONSE_CANCEL,
        "_OK", GTK_RESPONSE_OK, NULL);
    gtk_window_set_default_size(GTK_WINDOW(dlg), 480, 120);
    grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 8);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 12);

    lbl_c = gtk_label_new("Обчислення (compute):");
    lbl_v = gtk_label_new("Перегляд (viewer):");
    gtk_label_set_xalign(GTK_LABEL(lbl_c), 0.0);
    gtk_label_set_xalign(GTK_LABEL(lbl_v), 0.0);
    e_compute = gtk_entry_new();
    e_viewer = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(e_compute), app->config.compute_path);
    gtk_entry_set_text(GTK_ENTRY(e_viewer), app->config.viewer_path);

    gtk_grid_attach(GTK_GRID(grid), lbl_c, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), e_compute, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), lbl_v, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), e_viewer, 1, 1, 1, 1);
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dlg))),
                       grid, TRUE, TRUE, 0);
    gtk_widget_show_all(dlg);

    resp = gtk_dialog_run(GTK_DIALOG(dlg));
    if (resp == GTK_RESPONSE_OK) {
        strncpy(app->config.compute_path,
                gtk_entry_get_text(GTK_ENTRY(e_compute)), PATH_MAX_GOLD - 1);
        strncpy(app->config.viewer_path,
                gtk_entry_get_text(GTK_ENTRY(e_viewer)), PATH_MAX_GOLD - 1);
        app->config.compute_path[PATH_MAX_GOLD - 1] = '\0';
        app->config.viewer_path[PATH_MAX_GOLD - 1] = '\0';
        if (!programs_executable(app->config.compute_path)) {
            show_error(GTK_WINDOW(app->window), "Опції",
                       "Шлях compute недоступний — зміни не збережено.");
        } else if (!programs_executable(app->config.viewer_path)) {
            show_error(GTK_WINDOW(app->window), "Опції",
                       "Шлях viewer недоступний — зміни не збережено.");
        } else {
            gold_config_save(GOLD_CONF_FILE, &app->config);
            show_info(GTK_WINDOW(app->window), "Опції", "Збережено у config/gold.conf");
        }
    }
    gtk_widget_destroy(dlg);
}

static const char *result_modes[] = {
    "acf1", "acf2", "ccf1", "ccf2", "srt1", "srt2", "usrt1", "usrt2"
};
static const char *result_files[] = {
    "acf_1.txt", "acf_2.txt", "ccf_1.txt", "ccf_2.txt",
    "srt_1.txt", "srt_2.txt", "usrt_1.txt", "usrt_2.txt"
};

static void on_result(GtkWidget *w, gpointer data)
{
    GoldApp *app = data;
    int idx = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(w), "result_idx"));
    char *vargv[4];
    char modebuf[32];
    char err[512];

    if (idx < 0 || idx >= 8)
        return;
    if (!indication_result_file_exists(result_files[idx])) {
        show_info(GTK_WINDOW(app->window), "Результати",
                  "Спочатку виконайте обчислення (START).");
        return;
    }
    if (!programs_executable(app->config.viewer_path)) {
        show_error(GTK_WINDOW(app->window), "Перегляд",
                   "gold_viewer не знайдено. Зберіть: make");
        return;
    }
    snprintf(modebuf, sizeof modebuf, "--mode=%s", result_modes[idx]);
    vargv[0] = modebuf;
    vargv[1] = NULL;
    if (programs_spawn_path_argv(app->config.viewer_path, vargv, NULL,
                                 err, sizeof err) < 0)
        show_error(GTK_WINDOW(app->window), "Перегляд", err);
}

void gui_handlers_connect(GoldApp *app)
{
    GtkWidget *btn_manual, *btn_info;
    int i;

    btn_manual = GTK_WIDGET(g_object_get_data(G_OBJECT(app->window), "btn_manual"));
    btn_info = GTK_WIDGET(g_object_get_data(G_OBJECT(app->window), "btn_info"));

    g_signal_connect(btn_manual, "clicked", G_CALLBACK(on_manual), app);
    g_signal_connect(btn_info, "clicked", G_CALLBACK(on_info), app);
    g_signal_connect(app->btn_start, "clicked", G_CALLBACK(on_start), app);
    g_signal_connect(app->btn_finish, "clicked", G_CALLBACK(on_finish), app);
    g_signal_connect(app->btn_reset, "clicked", G_CALLBACK(on_reset), app);
    g_signal_connect(app->btn_print, "clicked", G_CALLBACK(on_print), app);
    g_signal_connect(app->btn_options, "clicked", G_CALLBACK(on_options), app);

    g_signal_connect(app->combo_n, "changed", G_CALLBACK(on_params_changed), app);
    g_signal_connect(app->combo_corr, "changed", G_CALLBACK(on_params_changed), app);
    g_signal_connect(app->combo_crit, "changed", G_CALLBACK(on_params_changed), app);
    g_signal_connect(app->combo_k, "changed", G_CALLBACK(on_params_changed), app);

    for (i = 0; i < 8; i++) {
        g_object_set_data(G_OBJECT(app->btn_result[i]), "result_idx",
                          GINT_TO_POINTER(i));
        g_signal_connect(app->btn_result[i], "clicked", G_CALLBACK(on_result), app);
    }
}
