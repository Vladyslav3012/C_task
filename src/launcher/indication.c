#include "indication.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static const char *RESULT_FILES[] = {
    "acf_1.txt", "acf_2.txt", "ccf_1.txt", "ccf_2.txt",
    "srt_1.txt", "srt_2.txt", "usrt_1.txt", "usrt_2.txt",
    NULL
};

static void set_entry(GtkWidget *entry, const char *text)
{
    gtk_entry_set_text(GTK_ENTRY(entry), text ? text : "");
}

static int parse_json_string(const char *json, const char *key, char *out, size_t outlen)
{
    char pattern[64];
    const char *p, *q;
    snprintf(pattern, sizeof pattern, "\"%s\"", key);
    p = strstr(json, pattern);
    if (!p)
        return -1;
    p = strchr(p + strlen(pattern), '"');
    if (!p)
        return -1;
    p++;
    q = strchr(p, '"');
    if (!q)
        return -1;
    if ((size_t)(q - p) >= outlen)
        return -1;
    memcpy(out, p, (size_t)(q - p));
    out[q - p] = '\0';
    return 0;
}

void indication_set_defaults(GoldApp *app)
{
    set_entry(app->entry_prim_b, "10_Б");
    set_entry(app->entry_prim_o, "2_Б");
    set_entry(app->entry_pairs_b, "10_пар");
    set_entry(app->entry_pairs_o, "2_пар");
    set_entry(app->entry_ens1, "16");
    set_entry(app->entry_ens2, "2");
    set_entry(app->entry_ens3, "+/-");
    app->results_ready = 0;
    indication_update_result_buttons(app);
}

void indication_load_summary(GoldApp *app)
{
    FILE *f;
    char buf[4096];
    char v[64];
    size_t n;

    f = fopen(SUMMARY_FILE, "r");
    if (!f)
        return;
    n = fread(buf, 1, sizeof buf - 1, f);
    buf[n] = '\0';
    fclose(f);

    if (parse_json_string(buf, "prim_base", v, sizeof v) == 0)
        set_entry(app->entry_prim_b, v);
    if (parse_json_string(buf, "prim_opt", v, sizeof v) == 0)
        set_entry(app->entry_prim_o, v);
    if (parse_json_string(buf, "pairs_base", v, sizeof v) == 0)
        set_entry(app->entry_pairs_b, v);
    if (parse_json_string(buf, "pairs_opt", v, sizeof v) == 0)
        set_entry(app->entry_pairs_o, v);
    if (parse_json_string(buf, "ens1", v, sizeof v) == 0)
        set_entry(app->entry_ens1, v);
    if (parse_json_string(buf, "ens2", v, sizeof v) == 0)
        set_entry(app->entry_ens2, v);
    if (parse_json_string(buf, "ens3", v, sizeof v) == 0)
        set_entry(app->entry_ens3, v);

    app->results_ready = 1;
    indication_update_result_buttons(app);
}

void indication_clear_output_files(void)
{
    int i;
    char path[128];
    for (i = 0; RESULT_FILES[i]; i++) {
        snprintf(path, sizeof path, "output/%s", RESULT_FILES[i]);
        remove(path);
    }
    remove(SUMMARY_FILE);
    remove("output/report.txt");
}

int indication_result_file_exists(const char *basename)
{
    char path[128];
    FILE *f;
    snprintf(path, sizeof path, "output/%s", basename);
    f = fopen(path, "r");
    if (!f)
        return 0;
    fclose(f);
    return 1;
}

void indication_update_result_buttons(GoldApp *app)
{
    static const char *names[] = {
        "acf_1.txt", "acf_2.txt", "ccf_1.txt", "ccf_2.txt",
        "srt_1.txt", "srt_2.txt", "usrt_1.txt", "usrt_2.txt",
    };
    int i;
    for (i = 0; i < 8; i++)
        gtk_widget_set_sensitive(app->btn_result[i],
                               indication_result_file_exists(names[i]));
}
