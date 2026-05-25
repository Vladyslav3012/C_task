/*
 * Перегляд файлів результатів GOLD (GTK3).
 * Запуск: gold_viewer --mode=acf1
 */
#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static const struct {
    const char *mode;
    const char *file;
    const char *title;
} MODES[] = {
    { "acf1",  "output/acf_1.txt",  "АКФ_1" },
    { "acf2",  "output/acf_2.txt",  "АКФ_2" },
    { "ccf1",  "output/ccf_1.txt",  "ВКФ_1" },
    { "ccf2",  "output/ccf_2.txt",  "ВКФ_2" },
    { "srt1",  "output/srt_1.txt",  "SRT_1" },
    { "srt2",  "output/srt_2.txt",  "SRT_2" },
    { "usrt1", "output/usrt_1.txt", "USRT_1" },
    { "usrt2", "output/usrt_2.txt", "USRT_2" },
    { NULL, NULL, NULL }
};

static const char *parse_mode(int argc, char **argv)
{
    int i;
    for (i = 1; i < argc; i++) {
        if (strncmp(argv[i], "--mode=", 7) == 0)
            return argv[i] + 7;
        if (strcmp(argv[i], "--mode") == 0 && i + 1 < argc)
            return argv[i + 1];
    }
    return "acf1";
}

static void load_file_to_buffer(const char *path, char **out, gsize *len)
{
    FILE *f;
    char line[256];
    GString *s = g_string_new(NULL);
    f = fopen(path, "r");
    if (!f) {
        g_string_printf(s, "Файл не знайдено: %s", path);
        *out = g_string_free(s, FALSE);
        *len = strlen(*out);
        return;
    }
    while (fgets(line, sizeof line, f))
        g_string_append(s, line);
    fclose(f);
    *out = g_string_free(s, FALSE);
    *len = strlen(*out);
}

int main(int argc, char **argv)
{
    const char *mode_arg;
    const char *file = "output/acf_1.txt";
    const char *title = "АКФ_1";
    GtkWidget *win, *scroll, *view;
    GtkTextBuffer *buf;
    char *text;
    gsize len;
    int i;

    mode_arg = parse_mode(argc, argv);
    for (i = 0; MODES[i].mode; i++) {
        if (strcmp(MODES[i].mode, mode_arg) == 0) {
            file = MODES[i].file;
            title = MODES[i].title;
            break;
        }
    }

    gtk_init(&argc, &argv);
    win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    {
        char wtitle[128];
        snprintf(wtitle, sizeof wtitle, "GOLD — %s", title);
        gtk_window_set_title(GTK_WINDOW(win), wtitle);
    }
    gtk_window_set_default_size(GTK_WINDOW(win), 520, 400);
    g_signal_connect(win, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    scroll = gtk_scrolled_window_new(NULL, NULL);
    view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(view), GTK_WRAP_NONE);
    gtk_container_add(GTK_CONTAINER(scroll), view);
    gtk_container_add(GTK_CONTAINER(win), scroll);

    load_file_to_buffer(file, &text, &len);
    buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
    gtk_text_buffer_set_text(buf, text, (gint)len);
    g_free(text);

    gtk_widget_show_all(win);
    gtk_main();
    return 0;
}
