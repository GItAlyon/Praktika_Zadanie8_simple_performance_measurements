#define _CRT_SECURE_NO_WARNINGS
#include "benchmark.h"
#include "lockfree_hash_task3.h"
#include "rbtree_task5.h"
#include <iostream>
#include <iomanip>
#include <random>
#include <vector>
#include <string>
#include <windows.h>

// ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ
// Инициализация глобальных переменных для хранения результатов
BenchmarkResult g_hash_result = { "Хеш-таблица", 0, 0, 0, 0, 0 };
BenchmarkResult g_lockfree_result = { "Неблокирующая HT", 0, 0, 0, 0, 0 };
BenchmarkResult g_rbtree_result = { "Красно-чёрное дерево", 0, 0, 0, 0, 0 };
SortResult g_quick_sort_result = { "Быстрая сортировка", 0, 0, 0 };
SortResult g_insertion_sort_result = { "Сортировка вставками", 0, 0, 0 };

// РЕАЛИЗАЦИЯ ТАЙМЕРА
// Возвращает время в секундах
double Timer::elapsed() const {
    auto end_time = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double>(end_time - start_time).count();
}

// Возвращает время в миллисекундах
double Timer::elapsed_ms() const {
    auto end_time = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(end_time - start_time).count();
}

// РЕАЛИЗАЦИЯ БЕНЧМАРКА

// Измерение времени выполнения функции (один запуск)
double Benchmark::measureTime(const std::function<void()>& func) {
    Timer timer;
    func();
    return timer.elapsed_ms();
}

// Многократное измерение с усреднением (отбрасываем выбросы)
double Benchmark::measureTimeAvg(const std::function<void()>& func, int repeats) {
    std::vector<double> times;
    for (int i = 0; i < repeats; i++) {
        times.push_back(measureTime(func));
    }
    std::sort(times.begin(), times.end());
    double sum = 0;
    for (int i = 1; i < repeats - 1; i++) {
        sum += times[i];
    }
    return sum / (repeats - 2);  // Усредняем без max и min
}

// Печать результатов для структур данных
void Benchmark::printResults(const std::vector<BenchmarkResult>& results) {
    std::cout << "\n  Сводная таблица результатов (10000 элементов):\n";
    std::cout << "  +---------------------+-------------+-------------+-------------+-------------+\n";
    std::cout << "  | Структура           | Вставка(мс) | Поиск(мс)   | Удаление(мс)| Всего(мс)   |\n";
    std::cout << "  +---------------------+-------------+-------------+-------------+-------------+\n";

    for (const auto& r : results) {
        std::cout << "  | " << std::left << std::setw(19) << r.name << " | "
            << std::right << std::setw(11) << std::fixed << std::setprecision(3) << r.insert_time_ms << " | "
            << std::setw(11) << r.lookup_time_ms << " | "
            << std::setw(11) << r.delete_time_ms << " | "
            << std::setw(11) << r.total_time_ms << " |\n";
    }
    std::cout << "  +---------------------+-------------+-------------+-------------+-------------+\n";
}

// Печать результатов для сортировок
void Benchmark::printSortResults(const std::vector<SortResult>& results) {
    std::cout << "\n  Результаты сортировки:\n";
    std::cout << "  +---------------------+-------------+-----------------+\n";
    std::cout << "  | Алгоритм            | Время (мс)  | мкс/элемент     |\n";
    std::cout << "  +---------------------+-------------+-----------------+\n";

    for (const auto& r : results) {
        std::cout << "  | " << std::left << std::setw(19) << r.name << " | "
            << std::right << std::setw(11) << std::fixed << std::setprecision(3) << r.time_ms << " | "
            << std::setw(15) << std::fixed << std::setprecision(3) << r.us_per_element << " |\n";
    }
    std::cout << "  +---------------------+-------------+-----------------+\n";
}

// АЛГОРИТМЫ СОРТИРОВКИ

// Сортировка вставками (O(n^2)) - простой, но медленный для больших массивов
static void insertion_sort(std::vector<int>& arr) {
    int n = arr.size();
    for (int i = 1; i < n; i++) {
        int key = arr[i];
        int j = i - 1;
        // Сдвигаем элементы, пока не найдём место для вставки
        while (j >= 0 && arr[j] > key) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}

// Быстрая сортировка (O(n log n)) - эффективный алгоритм "разделяй и властвуй"
static void quick_sort(std::vector<int>& arr, int low, int high) {
    if (low < high) {
        // Выбираем опорный элемент (pivot) - последний элемент
        int pivot = arr[high];
        int i = low - 1;
        // Разбиваем массив относительно pivot
        for (int j = low; j < high; j++) {
            if (arr[j] <= pivot) {
                i++;
                std::swap(arr[i], arr[j]);
            }
        }
        std::swap(arr[i + 1], arr[high]);
        int pi = i + 1;
        // Рекурсивно сортируем левую и правую части
        quick_sort(arr, low, pi - 1);
        quick_sort(arr, pi + 1, high);
    }
}

// Обёртка для быстрой сортировки (удобный интерфейс)
static void quick_sort_wrapper(std::vector<int>& arr) {
    quick_sort(arr, 0, arr.size() - 1);
}

// БЕНЧМАРК СОРТИРОВОК

extern "C" void run_sorting_benchmarks() {
    printf("\n  БЕНЧМАРК 4: АЛГОРИТМЫ СОРТИРОВКИ\n");

    // Размеры массивов для тестирования
    const int sizes[] = { 1000, 5000, 10000, 50000, 100000 };
    const char* size_names[] = { "1 000", "5 000", "10 000", "50 000", "100 000" };

    LARGE_INTEGER frequency, start, end;
    QueryPerformanceFrequency(&frequency);

    printf("\n  Сравнение быстрой сортировки и сортировки вставками:\n");
    printf("  +-----------+------------------+------------------+------------------+\n");
    printf("  | Размер    | Быстрая (мс)     | Вставками (мс)   | Ускорение        |\n");
    printf("  +-----------+------------------+------------------+------------------+\n");

    for (int idx = 0; idx < 5; idx++) {
        int n = sizes[idx];

        // Создаём случайный массив
        std::vector<int> data1(n);
        std::vector<int> data2(n);
        std::random_device rd;                        // Генератор случайных чисел
        std::mt19937 gen(rd());                       // Вихрь Мерсенна
        std::uniform_int_distribution<> dis(0, n * 10);  // Равномерное распределение

        for (int i = 0; i < n; i++) {
            data1[i] = dis(gen);
            data2[i] = data1[i];
        }

        // 1. БЫСТРАЯ СОРТИРОВКА
        std::vector<int> quick_data = data1;
        QueryPerformanceCounter(&start);
        quick_sort_wrapper(quick_data);
        QueryPerformanceCounter(&end);
        double quick_time = (double)(end.QuadPart - start.QuadPart) / frequency.QuadPart * 1000.0;

        // 2. СОРТИРОВКА ВСТАВКАМИ
        std::vector<int> insert_data = data2;
        QueryPerformanceCounter(&start);
        insertion_sort(insert_data);
        QueryPerformanceCounter(&end);
        double insert_time = (double)(end.QuadPart - start.QuadPart) / frequency.QuadPart * 1000.0;

        // Сохраняем результаты для 10000 элементов (для итогового сравнения)
        if (n == 10000) {
            g_quick_sort_result.name = "Быстрая сортировка";
            g_quick_sort_result.size = n;
            g_quick_sort_result.time_ms = quick_time;
            g_quick_sort_result.us_per_element = quick_time / n * 1000;

            g_insertion_sort_result.name = "Сортировка вставками";
            g_insertion_sort_result.size = n;
            g_insertion_sort_result.time_ms = insert_time;
            g_insertion_sort_result.us_per_element = insert_time / n * 1000;
        }

        // Вычисляем ускорение
        double speedup = (insert_time > 0) ? insert_time / quick_time : 0;

        printf("  | %-9s | %16.3f | %16.3f | %16.2fx |\n",
            size_names[idx], quick_time, insert_time, speedup);
    }
    printf("  +-----------+------------------+------------------+------------------+\n");

    // Анализ результатов
    printf("\n  Анализ:\n");
    if (g_quick_sort_result.time_ms > 0 && g_insertion_sort_result.time_ms > 0) {
        double speedup = g_insertion_sort_result.time_ms / g_quick_sort_result.time_ms;
        printf("  - Быстрая сортировка быстрее сортировки вставками в %.2f раза (при %d элементах)\n",
            speedup, g_quick_sort_result.size);
    }

    // Теоретическая сложность
    printf("\n  Теоретическая сложность:\n");
    printf("  - Быстрая сортировка: O(n log n) ~ %.2f операций\n", 10000 * log2(10000));
    printf("  - Сортировка вставками: O(n^2) ~ %.2f операций\n", 10000.0 * 10000.0);
    printf("  - Разница в теории: в %.2f раз\n", 10000.0 * 10000.0 / (10000 * log2(10000)));
}

// БЕНЧМАРК НЕБЛОКИРУЮЩЕЙ ХЕШ-ТАБЛИЦЫ (ЗАДАНИЕ 3)

extern "C" void benchmark_lockfree_hash_task3_int(int n) {
    printf("\n --- Неблокирующая хеш-таблица (Задание 3) - %d элементов ---\n", n);

    // Создаём тестовые данные
    std::vector<std::string> keys;
    std::vector<int> values;
    keys.reserve(n);
    values.reserve(n);

    for (int i = 0; i < n; i++) {
        char key[32];
        sprintf(key, "key_%d", i);
        keys.push_back(key);
        values.push_back(i * 10);
    }

    LARGE_INTEGER frequency, start, end;
    QueryPerformanceFrequency(&frequency);

    HashTableLockFree ht;

    // 1. ЗАМЕР ВСТАВКИ
    QueryPerformanceCounter(&start);
    for (int i = 0; i < n; i++) {
        int* val = new int(values[i]);  // Выделяем память для значения
        ht.insert(keys[i].c_str(), val);
    }
    QueryPerformanceCounter(&end);
    double insert_time = (double)(end.QuadPart - start.QuadPart) / frequency.QuadPart * 1000.0;

    // 2. ЗАМЕР ПОИСКА
    QueryPerformanceCounter(&start);
    for (int i = 0; i < n; i++) {
        ht.lookup(keys[i].c_str());
    }
    QueryPerformanceCounter(&end);
    double lookup_time = (double)(end.QuadPart - start.QuadPart) / frequency.QuadPart * 1000.0;

    // Сохраняем результат для итогового сравнения
    if (n == 10000) {
        g_lockfree_result.name = "Неблокирующая HT";
        g_lockfree_result.insert_time_ms = insert_time;
        g_lockfree_result.lookup_time_ms = lookup_time;
        g_lockfree_result.size = n;
    }

    // Подсчёт заполненных бакетов для статистики
    size_t non_empty = ht.count_non_empty_buckets();

    // Выводим статистику до удаления
    printf("\n  Статистика хеш-таблицы (до удаления):\n");
    printf("  - Активных элементов: %zu\n", ht.size());
    printf("  - Логических элементов: %zu\n", ht.size_logical());
    printf("  - Заполненных бакетов: %zu / 1024 (%.2f%%)\n", non_empty, (double)non_empty / 1024 * 100.0);

    // 3. ЗАМЕР УДАЛЕНИЯ
    QueryPerformanceCounter(&start);
    for (int i = 0; i < n; i++) {
        ht.remove(keys[i].c_str());
    }
    QueryPerformanceCounter(&end);
    double delete_time = (double)(end.QuadPart - start.QuadPart) / frequency.QuadPart * 1000.0;

    // Сохраняем время удаления для итогового сравнения
    if (n == 10000) {
        g_lockfree_result.delete_time_ms = delete_time;
        g_lockfree_result.total_time_ms = insert_time + lookup_time + delete_time;
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
    printf("  - Активных элементов: %zu\n", ht.size());
    printf("  - Логических элементов: %zu\n", ht.size_logical());
    printf("  - Удалённых узлов: %zu\n", ht.size_logical() - ht.size());
}

// БЕНЧМАРК КРАСНО-ЧЁРНОГО ДЕРЕВА (ЗАДАНИЕ 5)

extern "C" void benchmark_rbtree_task5_int(int n) {
    printf("\n --- Красно-чёрное дерево (Задание 5) - %d элементов ---\n", n);

    // Создаём тестовые данные
    std::vector<int> keys(n);
    for (int i = 0; i < n; i++) keys[i] = i;

    // Перемешиваем ключи (чтобы дерево было сбалансированным)
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(keys.begin(), keys.end(), g);

    LARGE_INTEGER frequency, start, end;
    QueryPerformanceFrequency(&frequency);

    AssociativeArray<int, int> dict;

    // 1. ЗАМЕР ВСТАВКИ
    QueryPerformanceCounter(&start);
    for (int i = 0; i < n; i++) {
        dict.insert(keys[i], keys[i] * 10);
    }
    QueryPerformanceCounter(&end);
    double insert_time = (double)(end.QuadPart - start.QuadPart) / frequency.QuadPart * 1000.0;

    // 2. ЗАМЕР ПОИСКА
    QueryPerformanceCounter(&start);
    for (int i = 0; i < n; i++) {
        int val;
        dict.find(keys[i], val);
    }
    QueryPerformanceCounter(&end);
    double lookup_time = (double)(end.QuadPart - start.QuadPart) / frequency.QuadPart * 1000.0;

    // Сохраняем результат для итогового сравнения
    if (n == 10000) {
        g_rbtree_result.name = "Красно-чёрное дерево";
        g_rbtree_result.insert_time_ms = insert_time;
        g_rbtree_result.lookup_time_ms = lookup_time;
        g_rbtree_result.size = n;
    }

    // Выводим статистику до удаления
    printf("\n  Статистика дерева (до удаления):\n");
    printf("  - Размер: %zu элементов\n", dict.size());
    printf("  - Пусто: %s\n", dict.empty() ? "да" : "нет");

    // 3. ЗАМЕР УДАЛЕНИЯ
    QueryPerformanceCounter(&start);
    for (int i = 0; i < n; i++) {
        dict.remove(keys[i]);
    }
    QueryPerformanceCounter(&end);
    double delete_time = (double)(end.QuadPart - start.QuadPart) / frequency.QuadPart * 1000.0;

    // Сохраняем время удаления для итогового сравнения
    if (n == 10000) {
        g_rbtree_result.delete_time_ms = delete_time;
        g_rbtree_result.total_time_ms = insert_time + lookup_time + delete_time;
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
    printf("\n  Статистика дерева (после удаления):\n");
    printf("  - Размер: %zu элементов\n", dict.size());
    printf("  - Пусто: %s\n", dict.empty() ? "да" : "нет");
}