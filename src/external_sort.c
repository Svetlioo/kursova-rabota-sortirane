/*
 * Task 6.07.3 - External sorting of integers (External Merge Sort)
 * ----------------------------------------------------------------
 * Reads integers from an input file and writes them sorted in
 * ascending order into a NEW file, WITHOUT destroying the original.
 *
 * The in-memory buffer is limited to N integers (set on the command line).
 *
 * Algorithm:
 *   PHASE 1 (split):  read up to N numbers -> qsort in memory -> write to
 *                     temporary files "run_0.tmp", "run_1.tmp", ...
 *   PHASE 2 (merge):  k-way merge - on each step take the smallest "head"
 *                     among all runs and write it to the output.
 *
 * Compile:  gcc -Wall -O2 -o external_sort external_sort.c
 * Run:      ./external_sort <input> <output> <N> [-v]
 *   -v : verbose mode (prints every number; useful only for small inputs)
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
        fprintf(stderr, "Usage: %s <input> <output> <N> [-v]\n", argv[0]);
        return 1;
    }
    const char *input_path  = argv[1];
    const char *output_path = argv[2];
    long N = atol(argv[3]);
    int verbose = (argc == 5 && strcmp(argv[4], "-v") == 0);

    if (N <= 0) {
        fprintf(stderr, "[ERROR] N must be positive.\n");
        return 1;
    }

    printf("=========================================================\n");
    printf("  EXTERNAL MERGE SORT\n");
    printf("=========================================================\n");
    printf("[INFO] Input  : %s\n", input_path);
    printf("[INFO] Output : %s\n", output_path);
    printf("[INFO] N      = %ld numbers in buffer\n", N);
    printf("[INFO] Verbose: %s\n\n", verbose ? "YES" : "NO");

    clock_t t_start = clock();

    FILE *fin = fopen(input_path, "r");
    if (!fin) { perror("[ERROR] input"); return 1; }

    long *buffer = (long *)malloc(sizeof(long) * N);
    if (!buffer) { fprintf(stderr, "[ERROR] out of memory\n"); fclose(fin); return 1; }

    /* ---------- PHASE 1: SPLIT ---------- */
    printf("---------- PHASE 1: SPLIT ----------\n");
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
            printf("  [run %d] %ld numbers -> %s\n", run_count, count, run_name);
        }
        run_count++;
    }
    fclose(fin);
    free(buffer);

    double t1_sec = (double)(clock() - t1) / CLOCKS_PER_SEC;
    printf("[PHASE 1] %d runs, %ld total numbers, %.3f sec.\n\n",
           run_count, total_in, t1_sec);

    if (run_count == 0) {
        FILE *fout = fopen(output_path, "w");
        if (fout) fclose(fout);
        printf("[INFO] Empty input -> empty output.\n");
        return 0;
    }

    /* ---------- PHASE 2: K-WAY MERGE ---------- */
    printf("---------- PHASE 2: MERGE ----------\n");
    clock_t t2 = clock();

    FILE **runs  = (FILE **)malloc(sizeof(FILE *) * run_count);
    long  *heads = (long  *)malloc(sizeof(long)  * run_count);
    int   *alive = (int   *)malloc(sizeof(int)   * run_count);
    if (!runs || !heads || !alive) { fprintf(stderr,"out of memory\n"); return 1; }

    for (int i = 0; i < run_count; i++) {
        char run_name[64];
        snprintf(run_name, sizeof(run_name), "run_%d.tmp", i);
        runs[i] = fopen(run_name, "r");
        if (!runs[i]) { perror(run_name); return 1; }
        alive[i] = (fscanf(runs[i], "%ld", &heads[i]) == 1);
    }

    FILE *fout = fopen(output_path, "w");
    if (!fout) { perror("output"); return 1; }

    long step = 0;
    long progress_step = total_in / 10;   /* print progress every 10% */
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
            printf("  step %ld: %ld (from run %d)\n", step, min_val, min_idx);
        } else if (step % progress_step == 0) {
            printf("  progress: %ld / %ld (%.0f%%)\n",
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
    printf("[PHASE 2] wrote %ld numbers, %.3f sec.\n\n", step, t2_sec);

    /* ---------- CLEANUP ---------- */
    for (int i = 0; i < run_count; i++) {
        if (runs[i]) fclose(runs[i]);
        char run_name[64];
        snprintf(run_name, sizeof(run_name), "run_%d.tmp", i);
        remove(run_name);
    }
    free(runs); free(heads); free(alive);

    double t_total = (double)(clock() - t_start) / CLOCKS_PER_SEC;
    printf("=========================================================\n");
    printf("  DONE! %ld numbers sorted -> %s\n", step, output_path);
    printf("  Original '%s' is preserved.\n", input_path);
    printf("  Total time: %.3f sec.\n", t_total);
    printf("=========================================================\n");
    return 0;
}
