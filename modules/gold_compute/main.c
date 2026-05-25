/*
 * Заглушка обчислення GOLD_code_correlation_analysis.
 * Пише output/summary.json та файли результатів.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define gold_mkdir(p) _mkdir(p)
#else
#include <unistd.h>
#include <sys/stat.h>
#define gold_mkdir(p) mkdir(p, 0755)
#endif

static int n = 5, k = 33;
static char corr[32] = "periodic";
static char crit[32] = "srzn_skv";

static void parse_args(int argc, char **argv)
{
    int i;
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--n") == 0 && i + 1 < argc)
            n = atoi(argv[++i]);
        else if (strcmp(argv[i], "--corr") == 0 && i + 1 < argc)
            strncpy(corr, argv[++i], sizeof corr - 1);
        else if (strcmp(argv[i], "--crit") == 0 && i + 1 < argc)
            strncpy(crit, argv[++i], sizeof crit - 1);
        else if (strcmp(argv[i], "--k") == 0 && i + 1 < argc)
            k = atoi(argv[++i]);
    }
}

static int write_summary(int prim_b, int prim_o, int pairs_b, int pairs_o,
                         int ens1, int ens2)
{
    FILE *f = fopen("output/summary.json", "w");
    if (!f)
        return -1;
    fprintf(f,
            "{\n"
            "  \"prim_base\": \"%d_Б\",\n"
            "  \"prim_opt\": \"%d_Б\",\n"
            "  \"pairs_base\": \"%d_пар\",\n"
            "  \"pairs_opt\": \"%d_пар\",\n"
            "  \"ens1\": \"%d\",\n"
            "  \"ens2\": \"%d\",\n"
            "  \"ens3\": \"+/-\"\n"
            "}\n",
            prim_b, prim_o, pairs_b, pairs_o, ens1, ens2);
    fclose(f);
    return 0;
}

static int write_matrix(const char *path, const char *title, int seed)
{
    FILE *f;
    int i, j;
    f = fopen(path, "w");
    if (!f)
        return -1;
    fprintf(f, "# %s  n=%d k=%d corr=%s crit=%s\n", title, n, k, corr, crit);
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 8; j++)
            fprintf(f, "%6.3f ", ((seed + i * 3 + j * 7) % 17) / 16.0 - 0.5);
        fprintf(f, "\n");
    }
    fclose(f);
    return 0;
}

int main(int argc, char **argv)
{
    int prim_b, prim_o, pairs_b, pairs_o, ens1, ens2;

    parse_args(argc, argv);
    gold_mkdir("output");

#ifdef _WIN32
    Sleep(800);
#else
    sleep(1);
#endif

    prim_b = (n <= 5) ? 10 : n * 2;
    prim_o = (n <= 6) ? 2 : n / 2;
    pairs_b = prim_b;
    pairs_o = prim_o;
    ens1 = (1 << (n - 1));
    if (ens1 > 64)
        ens1 = 64;
    ens2 = k / 16;
    if (ens2 < 1)
        ens2 = 1;

    write_summary(prim_b, prim_o, pairs_b, pairs_o, ens1, ens2);
    write_matrix("output/acf_1.txt", "ACF_1", 1);
    write_matrix("output/acf_2.txt", "ACF_2", 2);
    write_matrix("output/ccf_1.txt", "CCF_1", 3);
    write_matrix("output/ccf_2.txt", "CCF_2", 4);
    write_matrix("output/srt_1.txt", "SRT_1", 5);
    write_matrix("output/srt_2.txt", "SRT_2", 6);
    write_matrix("output/usrt_1.txt", "USRT_1", 7);
    write_matrix("output/usrt_2.txt", "USRT_2", 8);

    printf("gold_compute: готово (n=%d, K=%d)\n", n, k);
    return 0;
}
