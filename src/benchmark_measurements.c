#define _CRT_SECURE_NO_WARNINGS
#include "benchmark.h"
#include "hash_table_task2.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>

// Объявления C++ функций
#ifdef __cplusplus
extern "C" {
#endif

    void benchmark_lockfree_hash_task3_int(int n);
    void benchmark_rbtree_task5_int(int n);

#ifdef __cplusplus
}
#endif

// БЕНЧМАРК ХЕШ-ТАБЛИЦЫ (ЗАДАНИЕ 2)

// Запуск одного бенчмарка для заданного количества элементов
void benchmark_hash_table_task2(int n) {
    printf("\n --- Хеш-таблица с цепочками (Задание 2) - %d элементов ---\n", n);

    // Создаём тестовые данные: ключи "key_0", "key_1", ...
    char** keys = (char**)malloc(n * sizeof(char*));
    int* values = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) {
        keys[i] = (char*)malloc(32);
        sprintf(keys[i], "key_%d", i);
        values[i] = i * 10;
    }

    LARGE_INTEGER frequency, start, end;
    QueryPerformanceFrequency(&frequency);  // Получаем частоту таймера

    HashTable* ht = create_hash_table(1024);

    // 1. ЗАМЕР ВСТАВКИ
    QueryPerformanceCounter(&start);
    for (int i = 0; i < n; i++) {
        hash_table_insert(&ht, keys[i], values[i]);
    }
    QueryPerformanceCounter(&end);
    double insert_time = (double)(end.QuadPart - start.QuadPart) / frequency.QuadPart * 1000.0;

    // 2. ЗАМЕР ПОИСКА
    QueryPerformanceCounter(&start);
    for (int i = 0; i < n; i++) {
        int val;
        hash_table_get(ht, keys[i], &val);
    }
    QueryPerformanceCounter(&end);
    double lookup_time = (double)(end.QuadPart - start.QuadPart) / frequency.QuadPart * 1000.0;

    // Сохраняем результат для итогового сравнения (только для 10000 элементов)
    if (n == 10000) {
        g_hash_result.name = "Хеш-таблица";
        g_hash_result.insert_time_ms = insert_time;
        g_hash_result.lookup_time_ms = lookup_time;
        g_hash_result.size = n;
    }

    // Выводим статистику до удаления
    int collisions;
    float avg_chain;
    hash_table_stats(ht, &collisions, &avg_chain);

    printf("\n  Статистика хеш-таблицы (до удаления):\n");
    printf("  - Размер: %d элементов\n", ht->size);
    printf("  - Ёмкость: %d бакетов\n", ht->capacity);
    printf("  - Коллизий: %d\n", collisions);
    printf("  - Средняя длина цепочки: %.2f\n", avg_chain);
    printf("  - Коэффициент заполнения: %.2f%%\n", (float)ht->size / ht->capacity * 100);

    // 3. ЗАМЕР УДАЛЕНИЯ
    QueryPerformanceCounter(&start);
    for (int i = 0; i < n; i++) {
        hash_table_remove(ht, keys[i]);
    }
    QueryPerformanceCounter(&end);
    double delete_time = (double)(end.QuadPart - start.QuadPart) / frequency.QuadPart * 1000.0;

    // Сохраняем время удаления для итогового сравнения
    if (n == 10000) {
        g_hash_result.delete_time_ms = delete_time;
        g_hash_result.total_time_ms = insert_time + lookup_time + delete_time;
    }

    // Вывод результатов
    printf("\n  Результаты:\n");
    printf("  +---------------------+-------------+-----------------+\n");
    printf("  | Операция            | Время (мс)  | мкс/элемент     |\n");
    printf("  +---------------------+-------------+-----------------+\n");
    printf("  | Вставка             | %11.3f | %15.3f |\n", insert_time, insert_time / n * 1000);
    printf("  | Поиск               | %11.3f | %15.3f |\n", lookup_time, lookup_time / n * 1000);
    printf("  | Удаление            | %11.3f | %15.3f |\n", delete_time, delete_time / n * 1000);
    printf("  +---------------------+-------------+-----------------+\n");
    printf("  | Всего               | %11.3f | %15.3f |\n",
        insert_time + lookup_time + delete_time,
        (insert_time + lookup_time + delete_time) / n * 1000);
    printf("  +---------------------+-------------+-----------------+\n");

    // Статистика после удаления
    printf("\n  Статистика хеш-таблицы (после удаления):\n");
    printf("  - Размер: %d элементов\n", ht->size);
    printf("  - Ёмкость: %d бакетов\n", ht->capacity);

    // Очистка памяти
    free_hash_table(ht);
    for (int i = 0; i < n; i++) {
        free(keys[i]);
    }
    free(keys);
    free(values);
}

// Запуск бенчмарков для разных размеров
void run_hash_benchmarks_task2() {
    printf("\n  БЕНЧМАРК 1: ХЕШ-ТАБЛИЦА С ЦЕПОЧКАМИ (ЗАДАНИЕ 2)\n");

    int sizes[] = { 10000, 50000 };
    int num_sizes = 2;  // Явно указываем количество

    for (int i = 0; i < num_sizes; i++) {
        benchmark_hash_table_task2(sizes[i]);
        if (i < num_sizes - 1) printf("\n");
    }
}

// СРАВНЕНИЕ ВСЕХ СТРУКТУР

void compare_all_structures() {
    printf("\n  СРАВНЕНИЕ ВСЕХ СТРУКТУР ДАННЫХ\n");

    printf("\n  Сравнение при 10000 элементах:\n");
    printf("  +---------------------+-------------+-------------+-------------+-------------+\n");
    printf("  | Структура           | Вставка(мс) | Поиск(мс)   | Удаление(мс)| Всего(мс)   |\n");
    printf("  +---------------------+-------------+-------------+-------------+-------------+\n");

    // Выводим результаты из глобальных переменных
    printf("  | %-19s | %11.3f | %11.3f | %11.3f | %11.3f |\n",
        g_hash_result.name.c_str(),
        g_hash_result.insert_time_ms,
        g_hash_result.lookup_time_ms,
        g_hash_result.delete_time_ms,
        g_hash_result.total_time_ms);

    printf("  | %-19s | %11.3f | %11.3f | %11.3f | %11.3f |\n",
        g_lockfree_result.name.c_str(),
        g_lockfree_result.insert_time_ms,
        g_lockfree_result.lookup_time_ms,
        g_lockfree_result.delete_time_ms,
        g_lockfree_result.total_time_ms);

    printf("  | %-19s | %11.3f | %11.3f | %11.3f | %11.3f |\n",
        g_rbtree_result.name.c_str(),
        g_rbtree_result.insert_time_ms,
        g_rbtree_result.lookup_time_ms,
        g_rbtree_result.delete_time_ms,
        g_rbtree_result.total_time_ms);

    printf("  +---------------------+-------------+-------------+-------------+-------------+\n");

    // Анализ результатов
    printf("\n  Анализ:\n");

    // Определяем самую быструю структуру для каждой операции
    double min_insert = g_hash_result.insert_time_ms;
    const char* best_insert = "Хеш-таблица";
    if (g_lockfree_result.insert_time_ms < min_insert) {
        min_insert = g_lockfree_result.insert_time_ms;
        best_insert = "Неблокирующая HT";
    }
    if (g_rbtree_result.insert_time_ms < min_insert) {
        min_insert = g_rbtree_result.insert_time_ms;
        best_insert = "Красно-чёрное дерево";
    }

    double min_lookup = g_hash_result.lookup_time_ms;
    const char* best_lookup = "Хеш-таблица";
    if (g_lockfree_result.lookup_time_ms < min_lookup) {
        min_lookup = g_lockfree_result.lookup_time_ms;
        best_lookup = "Неблокирующая HT";
    }
    if (g_rbtree_result.lookup_time_ms < min_lookup) {
        min_lookup = g_rbtree_result.lookup_time_ms;
        best_lookup = "Красно-чёрное дерево";
    }

    double min_delete = g_hash_result.delete_time_ms;
    const char* best_delete = "Хеш-таблица";
    if (g_lockfree_result.delete_time_ms < min_delete) {
        min_delete = g_lockfree_result.delete_time_ms;
        best_delete = "Неблокирующая HT";
    }
    if (g_rbtree_result.delete_time_ms < min_delete) {
        min_delete = g_rbtree_result.delete_time_ms;
        best_delete = "Красно-чёрное дерево";
    }

    printf("  - Лучшая по вставке: %s (%.3f мс)\n", best_insert, min_insert);
    printf("  - Лучшая по поиску: %s (%.3f мс)\n", best_lookup, min_lookup);
    printf("  - Лучшая по удалению: %s (%.3f мс)\n", best_delete, min_delete);

    // Расчёт ускорения
    double hash_total = g_hash_result.total_time_ms;
    double rbtree_total = g_rbtree_result.total_time_ms;

    if (hash_total < rbtree_total) {
        printf("  - Хеш-таблица быстрее КЧД в %.2f раза\n", rbtree_total / hash_total);
    }
    else {
        printf("  - КЧД быстрее хеш-таблицы в %.2f раза\n", hash_total / rbtree_total);
    }
}

// ГЛАВНАЯ ФУНКЦИЯ БЕНЧМАРКА

void run_all_benchmarks() {
    // Инициализируем результаты
    g_hash_result.name = "Хеш-таблица";
    g_hash_result.insert_time_ms = 0;
    g_hash_result.lookup_time_ms = 0;
    g_hash_result.delete_time_ms = 0;
    g_hash_result.total_time_ms = 0;

    g_lockfree_result.name = "Неблокирующая HT";
    g_lockfree_result.insert_time_ms = 0;
    g_lockfree_result.lookup_time_ms = 0;
    g_lockfree_result.delete_time_ms = 0;
    g_lockfree_result.total_time_ms = 0;

    g_rbtree_result.name = "Красно-чёрное дерево";
    g_rbtree_result.insert_time_ms = 0;
    g_rbtree_result.lookup_time_ms = 0;
    g_rbtree_result.delete_time_ms = 0;
    g_rbtree_result.total_time_ms = 0;

    // Бенчмарк задания 2
    run_hash_benchmarks_task2();

    // Бенчмарк задания 3 (10000 и 50000 элементов)
    printf("\n  БЕНЧМАРК 2: НЕБЛОКИРУЮЩАЯ ХЕШ-ТАБЛИЦА (ЗАДАНИЕ 3)\n");
    benchmark_lockfree_hash_task3_int(10000);
    printf("\n");
    benchmark_lockfree_hash_task3_int(50000);

    // Бенчмарк задания 5 (10000 и 50000 элементов)
    printf("\n  БЕНЧМАРК 3: КРАСНО-ЧЁРНОЕ ДЕРЕВО (ЗАДАНИЕ 5)\n");
    benchmark_rbtree_task5_int(10000);
    printf("\n");
    benchmark_rbtree_task5_int(50000);

    // Бенчмарк сортировок
    run_sorting_benchmarks();

    // Сравнение всех структур
    compare_all_structures();

    printf("\n  БЕНЧМАРК ЗАВЕРШЁН\n");
}