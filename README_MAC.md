# Курсова работа по Програмиране на C
## Задача 6.07.3 — Външно сортиране на цели числа (версия за macOS)

## Условие

Да се сортират в нарастващ ред цели числа, които се четат от зададен файл.
Буферът, в който се четат числата, е ограничен до N позиции (задава се при
стартиране на програмата). Сортираният файл да се запише като нов
(да не се унищожава оригиналът).

## Идея на решението

External Merge Sort на 2 фази:

1. Чета по N числа, сортирам ги в паметта и ги записвам в малки временни
   файлове (run_0.tmp, run_1.tmp, ...).
2. Сливам всички тези малки сортирани файлове в един голям сортиран файл,
   като на всяка стъпка взимам най-малкото число измежду главите им.

## Структура на проекта

```
kursova-rabota/
├── README.md             версия за Windows (за хлапака)
├── README_MAC.md         тази версия за macOS
├── src/
│   ├── external_sort.c   главната програма
│   └── generate.c        помощна — генерира файл със случайни числа
├── bin/
│   ├── external_sort     компилиран binary за Mac
│   └── generate
└── data/
    ├── input/            входни файлове
    └── output/           сортирани резултати
```

## Компилация (macOS)

На Mac `gcc` всъщност е `clang` под капака (Apple преименува командата),
което за нашите цели не променя нищо — флаговете и поведението са същите.

Ако `gcc --version` не работи, инсталирай Command Line Tools:
```bash
xcode-select --install
```

След това в терминала влез в папката на проекта:
```bash
cd /Users/djamistyle/Desktop/kursova-rabota
```

И компилирай:
```bash
gcc -Wall -O2 -o bin/external_sort src/external_sort.c
gcc -Wall -O2 -o bin/generate      src/generate.c
```

Минимална версия (без флагове):
```bash
gcc src/external_sort.c -o bin/external_sort
```

Флаговете:
- `-Wall` — всички предупреждения
- `-O2` — оптимизация (без него 10M числа отнемат 30+ секунди вместо 9)
- `-o bin/external_sort` — име и място на изхода
- `src/external_sort.c` — входният .c файл

## Стартиране

```bash
./bin/external_sort <вход> <изход> <N> [-v]
```

- `<вход>` — файлът с числата
- `<изход>` — името на новия сортиран файл
- `<N>` — размер на буфера (брой числа едновременно в паметта)
- `-v` — verbose, печата всяка стъпка (само за малки файлове)

Генератор:
```bash
./bin/generate <изходен_файл> <брой_числа>
```

## Примери

Малък файл (15 числа, вече е генериран):
```bash
./bin/external_sort data/input/input.txt data/output/sorted.txt 4 -v
cat data/output/sorted.txt
```

1 000 числа:
```bash
./bin/generate data/input/test_1k.txt 1000
./bin/external_sort data/input/test_1k.txt data/output/sorted_1k.txt 50
```

10 000 числа:
```bash
./bin/generate data/input/big_10k.txt 10000
./bin/external_sort data/input/big_10k.txt data/output/sorted_10k.txt 1000
```

100 000 числа:
```bash
./bin/generate data/input/big_100k.txt 100000
./bin/external_sort data/input/big_100k.txt data/output/sorted_100k.txt 10000
```

1 милион числа:
```bash
./bin/generate data/input/big_1m.txt 1000000
./bin/external_sort data/input/big_1m.txt data/output/sorted_1m.txt 100000
```

10 милиона числа (~94 MB файл):
```bash
./bin/generate data/input/big_10m.txt 10000000
./bin/external_sort data/input/big_10m.txt data/output/sorted_10m.txt 500000
```

100 милиона числа:
```bash
./bin/generate data/input/big_100m.txt 100000000
./bin/external_sort data/input/big_100m.txt data/output/sorted_100m.txt 1000000
```

Малък буфер, много парчета (1M числа, буфер 1000 → 1000 парчета):
```bash
./bin/external_sort data/input/big_1m.txt data/output/out.txt 1000
```

Голям буфер, само 2 парчета:
```bash
./bin/external_sort data/input/big_1m.txt data/output/out.txt 500000
```

Едно парче (когато N ≥ брой числа):
```bash
./bin/external_sort data/input/big_10k.txt data/output/out.txt 100000
```

## Проверка че изходът е сортиран

На Mac имаме `sort -c`, което прави това в една команда:
```bash
sort -n -c data/output/sorted_10m.txt && echo OK
```

Сравнение между нашия резултат и стандартния `sort` на системата:
```bash
sort -n data/input/big_1m.txt > /tmp/expected.txt
diff /tmp/expected.txt data/output/sorted_1m.txt && echo "Идентични"
```

## Бенчмарк (всичко в един ред)

```bash
time ./bin/external_sort data/input/big_10m.txt data/output/sorted_10m.txt 500000
```

## Измерени времена (Apple Silicon, -O2)

| Брой числа | N (буфер) | Парчета | Време |
|------------|-----------|---------|-------|
| 1 000      | 50        | 20      | <0.01 с |
| 10 000     | 1 000     | 10      | 0.015 с |
| 100 000    | 10 000    | 10      | 0.10 с |
| 1 000 000  | 100 000   | 10      | 0.93 с |
| 1 000 000  | 1 000     | 1000    | 4.5 с |
| 1 000 000  | 500 000   | 2       | 0.6 с |
| 10 000 000 | 500 000   | 20      | 9.08 с |

## Бърз rebuild + run

```bash
gcc -Wall -O2 -o bin/external_sort src/external_sort.c && \
gcc -Wall -O2 -o bin/generate src/generate.c && \
./bin/external_sort data/input/input.txt data/output/sorted.txt 4 -v
```

## Почистване

Изтриване на компилираното:
```bash
rm -f bin/external_sort bin/generate
```

Изтриване на всички генерирани данни:
```bash
rm -f data/output/* data/input/big_*.txt data/input/test_*.txt
```

Ако случайно остане временен файл (не би трябвало, програмата ги трие сама):
```bash
rm -f run_*.tmp
```

## Бележки специфични за Mac

- `clang` се представя като `gcc` — няма разлика в употребата.
- `time` показва три времена: `real` (реално), `user` (CPU потребителско),
  `sys` (CPU в kernel). За бенчмарк гледаме `real`.
- При първо стартиране на голям файл macOS може да поиска позволение за
  достъп до Desktop — даваме го от System Settings → Privacy → Files.
- Ако `bin/external_sort` дава "permission denied", изпълни:
  `chmod +x bin/external_sort`.

## Останалите подробности

За детайлно обяснение на алгоритъма (фаза 1, фаза 2 със стъпка по стъпка
проследяване, сложност, други варианти на решение) виж `README.md`.
Логиката е една и съща, разликата с Windows версията е само в командите.
