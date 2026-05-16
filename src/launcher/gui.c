/*
 * Графічне меню (GTK3) — запуск прикладних програм.
 * Оформлення за зразком Interface.png (панелі, кнопки START / RESET / FINISH).
 */

#include "programs.h"

#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

typedef struct {
    GtkWidget *window;
    GtkWidget *status_label;
    GtkWidget *progress;
    GtkWidget *programs_box;
    GtkWidget *count_label;
} App;

static const char *CSS =
    "window { background-color: #d8dce3; }"
    ".panel { background-color: #c5cad4; border: 1px solid #9aa3b0; "
    "         border-radius: 4px; padding: 8px; }"
    ".title { font-weight: bold; font-size: 11pt; }"
    "button.btn-manual { background: #c0392b; color: white; font-weight: bold; "
    "                    padding: 6px 12px; }"
    "button.btn-info { background: #2980b9; color: white; font-weight: bold; "
    "                   padding: 6px 12px; }"
    "button.btn-start { background: #27ae60; color: white; font-weight: bold; "
    "                    min-height: 36px; min-width: 120px; }"
    "button.btn-finish { background: #7f8c8d; color: white; font-weight: bold; "
    "                     min-height: 36px; min-width: 120px; }"
    "button.btn-reset { background: #e74c3c; color: white; font-weight: bold; }"
    "button.btn-print { background: #8e44ad; color: white; font-weight: bold; }"
    "button.btn-prog { background: #3498db; color: white; font-weight: bold; "
    "                  min-height: 44px; margin: 4px; }"
    "button.btn-prog:disabled { background: #95a5a6; }"
    "label.hint { color: #2c3e50; }";

static void apply_css(void)
{
    GtkCssProvider *p = gtk_css_provider_new();
    gtk_css_provider_load_from_data(p, CSS, -1, NULL);
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(p),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(p);
}

static void set_status(App *app, const char *text)
{
    gtk_label_set_text(GTK_LABEL(app->status_label), text);
}

static void progress_pulse(App *app, gboolean on)
{
    if (on)
        gtk_progress_bar_pulse(GTK_PROGRESS_BAR(app->progress));
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(app->progress), on ? 0.0 : 0.0);
    gtk_widget_set_sensitive(app->progress, on);
}

static void show_error(App *app, const char *title, const char *msg)
{
    GtkWidget *d = gtk_message_dialog_new(
        GTK_WINDOW(app->window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
        GTK_BUTTONS_OK, "%s", msg);
    gtk_window_set_title(GTK_WINDOW(d), title);
    gtk_dialog_run(GTK_DIALOG(d));
    gtk_widget_destroy(d);
    set_status(app, msg);
}

static void show_info(GtkWindow *parent, const char *title, const char *msg)
{
    GtkWidget *d = gtk_message_dialog_new(
        parent, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", msg);
    gtk_window_set_title(GTK_WINDOW(d), title);
    gtk_dialog_run(GTK_DIALOG(d));
    gtk_widget_destroy(d);
}

static int spawn_program(const Program *prog, char *errbuf, size_t errlen)
{
    pid_t pid;
    const char *home = getenv("LAUNCHER_HOME");

    if (!programs_executable(prog->path)) {
        snprintf(errbuf, errlen,
                 "Файл «%s» не знайдено або не виконуваний.", prog->path);
        return -1;
    }

    pid = fork();
    if (pid < 0) {
        snprintf(errbuf, errlen, "fork: %s", strerror(errno));
        return -1;
    }
    if (pid == 0) {
        if (home && home[0])
            (void)chdir(home);
        execl(prog->path, prog->path, (char *)NULL);
        fprintf(stderr, "exec %s: %s\n", prog->path, strerror(errno));
        _exit(127);
    }
    return (int)pid;
}

static void on_program_clicked(GtkButton *btn, gpointer user_data)
{
    App *app = user_data;
    const Program *prog = g_object_get_data(G_OBJECT(btn), "program");
    char err[512];
    char status[NAME_LEN + 64];

    if (!prog)
        return;

    progress_pulse(app, TRUE);
    snprintf(status, sizeof status, "Запуск: %s …", prog->name);
    set_status(app, status);

    if (spawn_program(prog, err, sizeof err) < 0) {
        progress_pulse(app, FALSE);
        show_error(app, "Помилка запуску", err);
        return;
    }

    progress_pulse(app, FALSE);
    snprintf(status, sizeof status, "Запущено: %s  (окреме вікно)", prog->name);
    set_status(app, status);
}

static void clear_box(GtkWidget *box)
{
    GList *children = gtk_container_get_children(GTK_CONTAINER(box));
    for (GList *l = children; l; l = l->next)
        gtk_widget_destroy(GTK_WIDGET(l->data));
    g_list_free(children);
}

static void rebuild_program_buttons(App *app)
{
    int n = programs_count();
    char buf[64];

    clear_box(app->programs_box);

    for (int i = 0; i < n; i++) {
        const Program *p = programs_get(i);
        GtkWidget *row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
        GtkWidget *btn = gtk_button_new_with_label(p->name);
        GtkWidget *desc = gtk_label_new(p->desc);
        gboolean ok = programs_executable(p->path);

        gtk_widget_set_halign(desc, GTK_ALIGN_START);
        gtk_widget_set_hexpand(desc, TRUE);
        gtk_label_set_xalign(GTK_LABEL(desc), 0.0);
        gtk_widget_set_name(btn, "btn-prog");
        gtk_style_context_add_class(gtk_widget_get_style_context(btn), "btn-prog");

        if (!ok)
            gtk_widget_set_sensitive(btn, FALSE);

        g_object_set_data(G_OBJECT(btn), "program", (gpointer)p);
        g_signal_connect(btn, "clicked", G_CALLBACK(on_program_clicked), app);

        gtk_box_pack_start(GTK_BOX(row), btn, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(row), desc, TRUE, TRUE, 0);
        gtk_box_pack_start(GTK_BOX(app->programs_box), row, FALSE, FALSE, 0);
    }

    snprintf(buf, sizeof buf, "Підключено програм: %d", n);
    gtk_label_set_text(GTK_LABEL(app->count_label), buf);

    if (n == 0)
        set_status(app, "Список порожній. Додайте записи у config/programs.conf і натисніть RESET.");
}

static void on_reset(GtkWidget *w, gpointer user_data)
{
    (void)w;
    App *app = user_data;

    if (programs_load(CONF_FILE) < 0)
        show_error(app, "Конфігурація", "Не вдалося прочитати config/programs.conf");
  else {
        char s[80];
        snprintf(s, sizeof s, "Оновлено. Програм у меню: %d", programs_count());
        set_status(app, s);
    }
    rebuild_program_buttons(app);
}

static void on_finish(GtkWidget *w, gpointer user_data)
{
    (void)w;
    (void)user_data;
    gtk_main_quit();
}

static void on_manual(GtkWidget *w, gpointer user_data)
{
    (void)w;
    show_info(GTK_WINDOW(((App *)user_data)->window), "USER MANUAL",
              "Система меню для запуску прикладних програм.\n\n"
              "1. Додайте програму в config/programs.conf:\n"
              "   Назва|шлях|опис\n\n"
              "2. Натисніть RESET — оновити список.\n\n"
              "3. Натисніть кнопку програми (синя) — запуск.\n\n"
              "4. FINISH — вихід з меню.\n\n"
              "Шляхи відносні до каталогу проєкту (LAUNCHER_HOME).");
}

static void on_info(GtkWidget *w, gpointer user_data)
{
    const char *home = getenv("LAUNCHER_HOME");
    char msg[512];

    (void)w;
    snprintf(msg, sizeof msg,
             "Програма [MENU_launcher], версія 1\n"
             "Мова реалізації: C (GTK3)\n\n"
             "Конфігурація: %s\n"
             "Каталог: %s\n"
             "Програм у меню: %d",
             CONF_FILE, home ? home : "(поточний каталог)", programs_count());
    show_info(GTK_WINDOW(((App *)user_data)->window), "INFO (осн. парам.)", msg);
}

static void on_print(GtkWidget *w, gpointer user_data)
{
    FILE *f;
    App *app = user_data;

    (void)w;
    f = fopen(OUTPUT_DIR "/programs_list.txt", "w");
    if (!f) {
        show_error(app, "PRINT", "Не вдалося створити output/programs_list.txt");
        return;
    }
    fprintf(f, "Список програм меню\n====================\n");
    for (int i = 0; i < programs_count(); i++) {
        const Program *p = programs_get(i);
        fprintf(f, "%d. %s\n   Шлях: %s\n   Опис: %s\n   Доступ: %s\n\n",
                i + 1, p->name, p->path, p->desc,
                programs_executable(p->path) ? "так" : "ні");
    }
    fclose(f);
    set_status(app, "Збережено: output/programs_list.txt");
    show_info(GTK_WINDOW(app->window), "PRINT",
              "Список збережено у файл output/programs_list.txt");
}

static GtkWidget *labeled_panel(const char *title)
{
    GtkWidget *frame = gtk_frame_new(NULL);
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    GtkWidget *lbl = gtk_label_new(title);

    gtk_widget_set_name(frame, "panel");
    gtk_style_context_add_class(gtk_widget_get_style_context(frame), "panel");
    gtk_label_set_xalign(GTK_LABEL(lbl), 0.0);
    gtk_widget_set_name(lbl, "title");
    gtk_style_context_add_class(gtk_widget_get_style_context(lbl), "title");

    gtk_frame_set_label_widget(GTK_FRAME(frame), lbl);
    gtk_container_add(GTK_CONTAINER(frame), box);
    return frame;
}

int main(int argc, char **argv)
{
    App app;
    GtkWidget *header, *vbox, *top_row, *mid_row, *bot_row;
    GtkWidget *panel_prog, *inner;
    GtkWidget *btn_manual, *btn_info, *btn_reset, *btn_print, *btn_finish;

    (void)argv;
    gtk_init(&argc, &argv);
    apply_css();

    programs_chdir_home();
    if (programs_load(CONF_FILE) < 0)
        fprintf(stderr, "Увага: %s не знайдено.\n", CONF_FILE);

    memset(&app, 0, sizeof app);

    app.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(app.window),
                         "Програма [MENU_launcher], версія 1, C language");
    gtk_window_set_default_size(GTK_WINDOW(app.window), 720, 520);
    gtk_window_set_position(GTK_WINDOW(app.window), GTK_WIN_POS_CENTER);
    g_signal_connect(app.window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);
    gtk_container_add(GTK_CONTAINER(app.window), vbox);

    /* Верхній ряд: USER MANUAL | заголовок | INFO */
    top_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    btn_manual = gtk_button_new_with_label("USER MANUAL");
    gtk_widget_set_name(btn_manual, "btn-manual");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_manual), "btn-manual");
    g_signal_connect(btn_manual, "clicked", G_CALLBACK(on_manual), &app);

    header = gtk_label_new("Система меню — запуск прикладних програм");
    gtk_widget_set_hexpand(header, TRUE);
    gtk_label_set_xalign(GTK_LABEL(header), 0.5);

    btn_info = gtk_button_new_with_label("INFO (осн. парам.)");
    gtk_widget_set_name(btn_info, "btn-info");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_info), "btn-info");
    g_signal_connect(btn_info, "clicked", G_CALLBACK(on_info), &app);

    gtk_box_pack_start(GTK_BOX(top_row), btn_manual, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(top_row), header, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(top_row), btn_info, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), top_row, FALSE, FALSE, 0);

    /* Панель програм */
    panel_prog = labeled_panel("Підключені прикладні програми (натисніть для запуску)");
    inner = gtk_bin_get_child(GTK_BIN(panel_prog));
    app.programs_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    app.count_label = gtk_label_new("");
    gtk_label_set_xalign(GTK_LABEL(app.count_label), 0.0);
    gtk_box_pack_start(GTK_BOX(inner), app.programs_box, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(inner), app.count_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), panel_prog, TRUE, TRUE, 0);

    rebuild_program_buttons(&app);

    /* Середній ряд: RESET | progress | FINISH */
    mid_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    btn_reset = gtk_button_new_with_label("RESET");
    gtk_widget_set_name(btn_reset, "btn-reset");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_reset), "btn-reset");
    g_signal_connect(btn_reset, "clicked", G_CALLBACK(on_reset), &app);

    app.progress = gtk_progress_bar_new();
    gtk_widget_set_hexpand(app.progress, TRUE);
    gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(app.progress), TRUE);
    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(app.progress), "0%");

    btn_finish = gtk_button_new_with_label("FINISH");
    gtk_widget_set_name(btn_finish, "btn-finish");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_finish), "btn-finish");
    g_signal_connect(btn_finish, "clicked", G_CALLBACK(on_finish), NULL);

    gtk_box_pack_start(GTK_BOX(mid_row), btn_reset, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(mid_row), app.progress, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(mid_row), btn_finish, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), mid_row, FALSE, FALSE, 0);

    /* Нижній ряд: PRINT | статус */
    bot_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    btn_print = gtk_button_new_with_label("PRINT");
    gtk_widget_set_name(btn_print, "btn-print");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_print), "btn-print");
    g_signal_connect(btn_print, "clicked", G_CALLBACK(on_print), &app);

    app.status_label = gtk_label_new("Готово до роботи.");
    gtk_widget_set_name(app.status_label, "hint");
    gtk_style_context_add_class(gtk_widget_get_style_context(app.status_label), "hint");
    gtk_label_set_xalign(GTK_LABEL(app.status_label), 0.0);
    gtk_widget_set_hexpand(app.status_label, TRUE);

    gtk_box_pack_start(GTK_BOX(bot_row), btn_print, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(bot_row), app.status_label, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), bot_row, FALSE, FALSE, 0);

    gtk_widget_show_all(app.window);
    gtk_main();
    return 0;
}
