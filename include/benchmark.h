#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <chrono>      // Для высокоточного измерения времени
#include <string>      // Для std::string
#include <vector>      // Для std::vector
#include <functional>  // Для std::function

// Класс для измерения времени с высокой точностью
class Timer {
private:
    std::chrono::high_resolution_clock::time_point start_time;  // Время начала

public:
    Timer() { start(); }                                        // Автоматический запуск
    void start() { start_time = std::chrono::high_resolution_clock::now(); }  // Запуск таймера
    double elapsed() const;                                     // Время в секундах
    double elapsed_ms() const;                                  // Время в миллисекундах
};

// Структура для хранения результатов бенчмарка структур данных
struct BenchmarkResult {
    std::string name;               // Название структуры
    double insert_time_ms;          // Время вставки (мс)
    double lookup_time_ms;          // Время поиска (мс)
    double delete_time_ms;          // Время удаления (мс)
    double total_time_ms;           // Общее время (мс)
    int size;                       // Количество элементов
};

// Структура для хранения результатов бенчмарка сортировок
struct SortResult {
    std::string name;               // Название алгоритма
    double time_ms;                 // Время сортировки (мс)
    int size;                       // Количество элементов
    double us_per_element;          // Микросекунд на элемент
};

// Класс для проведения бенчмарков
class Benchmark {
public:
    // Измерение времени выполнения функции
    static double measureTime(const std::function<void()>& func);
    // Многократное измерение с усреднением (отбрасываем выбросы)
    static double measureTimeAvg(const std::function<void()>& func, int repeats = 5);
    // Печать результатов для структур данных
    static void printResults(const std::vector<BenchmarkResult>& results);
    // Печать результатов для сортировок
    static void printSortResults(const std::vector<SortResult>& results);
};

// Объявление глобальных переменных для обмена данными между C и C++ кодом
#ifdef __cplusplus
extern "C" {
#endif

    extern BenchmarkResult g_hash_result;        // Результаты хеш-таблицы
    extern BenchmarkResult g_lockfree_result;    // Результаты неблокирующей HT
    extern BenchmarkResult g_rbtree_result;      // Результаты КЧД
    extern SortResult g_quick_sort_result;       // Результаты быстрой сортировки
    extern SortResult g_insertion_sort_result;   // Результаты сортировки вставками

    // Основные функции запуска бенчмарков
    void run_all_benchmarks();                   // Запуск всех бенчмарков
    void run_hash_benchmarks_task2();            // Бенчмарк хеш-таблицы
    void run_lockfree_benchmarks_task3();        // Бенчмарк неблокирующей HT
    void run_rbtree_benchmarks_task5();          // Бенчмарк КЧД
    void compare_all_structures();               // Сравнение всех структур

    // Функции для бенчмарка сортировок
    void run_sorting_benchmarks();

#ifdef __cplusplus
}
#endif

#endif // BENCHMARK_H