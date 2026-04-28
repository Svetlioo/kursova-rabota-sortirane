/*
 * Задача 6.07.3 - Външно сортиране на цели числа (External Merge Sort)
 * --------------------------------------------------------------------
 * Чете цели числа от входен файл и ги записва сортирани в нарастващ ред
 * в нов файл, БЕЗ да унищожава оригинала.
 *
 * Буферът в паметта е ограничен до N числа (задава се от командния ред).
 *
 * Алгоритъм:
 *   ФАЗА 1 (split):  четем по N числа -> qsort в паметта -> записваме във
 *                    временни файлове "run_0.tmp", "run_1.tmp", ...
 *   ФАЗА 2 (merge):  k-way merge - на всяка стъпка взимаме най-малката
 *                    "глава" измежду всички парчета и я записваме в изхода.
 *
 * Компилация:  gcc -Wall -O2 -o external_sort external_sort.c
 * Стартиране:  ./external_sort <вход> <изход> <N> [-v]
 *   -v : verbose режим (печата всяко число; полезно само за малки входове)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>

static int cmp_int(const void *a, const void *b) {
    long x = *(const long *)a;
    long y = *(const long *)b;
    if (x < y) return -1;
    if (x > y) return 1;
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 4 || argc > 5) {
        fprintf(stderr, "Употреба: %s <вход> <изход> <N> [-v]\n", argv[0]);
        return 1;
    }
    const char *input_path  = argv[1];
    const char *output_path = argv[2];
    long N = atol(argv[3]);
    int verbose = (argc == 5 && strcmp(argv[4], "-v") == 0);

    if (N <= 0) {
        fprintf(stderr, "[ГРЕШКА] N трябва да е положително.\n");
        return 1;
    }

    printf("=========================================================\n");
    printf("  ВЪНШНО СОРТИРАНЕ (External Merge Sort)\n");
    printf("=========================================================\n");
    printf("[ИНФО] Вход : %s\n", input_path);
    printf("[ИНФО] Изход: %s\n", output_path);
    printf("[ИНФО] N    = %ld числа в буфер\n", N);
    printf("[ИНФО] Verbose: %s\n\n", verbose ? "ДА" : "НЕ");

    clock_t t_start = clock();

    FILE *fin = fopen(input_path, "r");
    if (!fin) { perror("[ГРЕШКА] вход"); return 1; }

    long *buffer = (long *)malloc(sizeof(long) * N);
    if (!buffer) { fprintf(stderr, "[ГРЕШКА] няма памет\n"); fclose(fin); return 1; }

    /* ---------- ФАЗА 1: РАЗДЕЛЯНЕ ---------- */
    printf("---------- ФАЗА 1: РАЗДЕЛЯНЕ ----------\n");
    clock_t t1 = clock();

    int run_count = 0;
    long total_in = 0;
    long count;

    while (1) {
        count = 0;
        while (count < N && fscanf(fin, "%ld", &buffer[count]) == 1) count++;
        if (count == 0) break;

        qsort(buffer, count, sizeof(long), cmp_int);

        char run_name[64];
        snprintf(run_name, sizeof(run_name), "run_%d.tmp", run_count);
        FILE *frun = fopen(run_name, "w");
        if (!frun) { perror("run"); free(buffer); fclose(fin); return 1; }
        for (long i = 0; i < count; i++) fprintf(frun, "%ld\n", buffer[i]);
        fclose(frun);

        total_in += count;
        if (verbose || run_count < 3 || run_count % 100 == 0) {
            printf("  [парче %d] %ld числа -> %s\n", run_count, count, run_name);
        }
        run_count++;
    }
    fclose(fin);
    free(buffer);

    double t1_sec = (double)(clock() - t1) / CLOCKS_PER_SEC;
    printf("[ФАЗА 1] %d парчета, %ld общо числа, %.3f сек.\n\n",
           run_count, total_in, t1_sec);

    if (run_count == 0) {
        FILE *fout = fopen(output_path, "w");
        if (fout) fclose(fout);
        printf("[ИНФО] Празен вход -> празен изход.\n");
        return 0;
    }

    /* ---------- ФАЗА 2: K-WAY СЛИВАНЕ ---------- */
    printf("---------- ФАЗА 2: СЛИВАНЕ ----------\n");
    clock_t t2 = clock();

    FILE **runs  = (FILE **)malloc(sizeof(FILE *) * run_count);
    long  *heads = (long  *)malloc(sizeof(long)  * run_count);
    int   *alive = (int   *)malloc(sizeof(int)   * run_count);
    if (!runs || !heads || !alive) { fprintf(stderr,"памет\n"); return 1; }

    for (int i = 0; i < run_count; i++) {
        char run_name[64];
        snprintf(run_name, sizeof(run_name), "run_%d.tmp", i);
        runs[i] = fopen(run_name, "r");
        if (!runs[i]) { perror(run_name); return 1; }
        alive[i] = (fscanf(runs[i], "%ld", &heads[i]) == 1);
    }

    FILE *fout = fopen(output_path, "w");
    if (!fout) { perror("изход"); return 1; }

    long step = 0;
    long progress_step = total_in / 10;   /* печатаме на всеки 10% */
    if (progress_step < 1) progress_step = 1;

    while (1) {
        int min_idx = -1;
        long min_val = 0;
        for (int i = 0; i < run_count; i++) {
            if (alive[i] && (min_idx == -1 || heads[i] < min_val)) {
                min_val = heads[i];
                min_idx = i;
            }
        }
        if (min_idx == -1) break;

        fprintf(fout, "%ld\n", min_val);
        step++;

        if (verbose) {
            printf("  стъпка %ld: %ld (от парче %d)\n", step, min_val, min_idx);
        } else if (step % progress_step == 0) {
            printf("  напредък: %ld / %ld (%.0f%%)\n",
                   step, total_in, 100.0 * step / total_in);
        }

        if (fscanf(runs[min_idx], "%ld", &heads[min_idx]) != 1) {
            alive[min_idx] = 0;
            fclose(runs[min_idx]);
            runs[min_idx] = NULL;
        }
    }
    fclose(fout);

    double t2_sec = (double)(clock() - t2) / CLOCKS_PER_SEC;
    printf("[ФАЗА 2] записани %ld числа, %.3f сек.\n\n", step, t2_sec);

    /* ---------- ПОЧИСТВАНЕ ---------- */
    for (int i = 0; i < run_count; i++) {
        if (runs[i]) fclose(runs[i]);
        char run_name[64];
        snprintf(run_name, sizeof(run_name), "run_%d.tmp", i);
        remove(run_name);
    }
    free(runs); free(heads); free(alive);

    double t_total = (double)(clock() - t_start) / CLOCKS_PER_SEC;
    printf("=========================================================\n");
    printf("  ГОТОВО! %ld числа сортирани -> %s\n", step, output_path);
    printf("  Оригиналът '%s' е запазен.\n", input_path);
    printf("  Общо време: %.3f сек.\n", t_total);
    printf("=========================================================\n");
    return 0;
}
