/*
 * Генератор на тестови файлове със случайни цели числа.
 * Компилация: gcc -O2 -o generate generate.c
 * Стартиране: ./generate <изходен_файл> <брой_числа>
 * Пример:     ./generate big.txt 1000000
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Употреба: %s <файл> <брой>\n", argv[0]);
        return 1;
    }
    const char *path = argv[1];
    long count = atol(argv[2]);

    FILE *f = fopen(path, "w");
    if (!f) { perror("fopen"); return 1; }

    srand((unsigned)time(NULL));
    for (long i = 0; i < count; i++) {
        /* Числа от 0 до ~1 милиард */
        long n = ((long)rand() << 15) ^ rand();
        if (n < 0) n = -n;
        n %= 1000000000L;
        fprintf(f, "%ld\n", n);
    }
    fclose(f);
    printf("Генерирани %ld числа във '%s'\n", count, path);
    return 0;
}
