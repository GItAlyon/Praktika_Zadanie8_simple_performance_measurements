#include "benchmark.h"
#include <iostream>
#include <iomanip>
#include <windows.h>

// Объявляем глобальные переменные из benchmark.cpp
extern BenchmarkResult g_hash_result;
extern BenchmarkResult g_lockfree_result;
extern BenchmarkResult g_rbtree_result;
extern SortResult g_quick_sort_result;
extern SortResult g_insertion_sort_result;

// Вспомогательная функция для поиска минимума (чтобы избежать конфликта с макросом min)
template<typename T>
T my_min(T a, T b, T c) {
    T min_val = a;
    if (b < min_val) min_val = b;
    if (c < min_val) min_val = c;
    return min_val;
}

int main() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    // Вывод заголовка программы
    std::cout << "\n";
    std::cout << "                БЕНЧМАРК ПРОИЗВОДИТЕЛЬНОСТИ                  \n";
    std::cout << "                                                             \n";
    std::cout << "      Сравнение структур данных из заданий 2, 3 и 5          \n";
    std::cout << "                                                             \n";
    std::cout << "      • Хеш-таблица с цепочками (задание 2)                 \n";
    std::cout << "      • Неблокирующая хеш-таблица (задание 3)               \n";
    std::cout << "      • Красно-чёрное дерево (задание 5)                    \n";
    std::cout << "      • Быстрая сортировка vs Сортировка вставками          \n";
    std::cout << "                                                             \n";

    // Запуск всех бенчмарков
    run_all_benchmarks();

    // ИТОГОВОЕ СРАВНЕНИЕ
    std::cout << "\n";
    std::cout << "               ИТОГОВОЕ СРАВНЕНИЕ         \n\n";

    // Проверяем, что данные получены
    if (g_hash_result.total_time_ms == 0 ||
        g_lockfree_result.total_time_ms == 0 ||
        g_rbtree_result.total_time_ms == 0) {
        std::cout << "  ОШИБКА: Данные бенчмарков не получены!\n";
        std::cout << "  Проверьте, что все тесты были запущены.\n";
    }
    else {
        // 1. ТАБЛИЦА ДЛЯ ХЕШ-ТАБЛИЦЫ И КЧД
        std::cout << "  1. Результаты при 10000 элементах:\n";
        std::cout << "  +---------------------+-------------+-------------+-------------+-------------+\n";
        std::cout << "  | Структура           | Вставка(мс) | Поиск(мс)   | Удаление(мс)| Всего(мс)   |\n";
        std::cout << "  +---------------------+-------------+-------------+-------------+-------------+\n";

        // Форматированный вывод с выравниванием
        std::cout << "  | " << std::left << std::setw(19) << g_hash_result.name << " | "
            << std::right << std::setw(11) << std::fixed << std::setprecision(3) << g_hash_result.insert_time_ms << " | "
            << std::setw(11) << g_hash_result.lookup_time_ms << " | "
            << std::setw(11) << g_hash_result.delete_time_ms << " | "
            << std::setw(11) << g_hash_result.total_time_ms << " |\n";

        std::cout << "  | " << std::left << std::setw(19) << g_lockfree_result.name << " | "
            << std::right << std::setw(11) << std::fixed << std::setprecision(3) << g_lockfree_result.insert_time_ms << " | "
            << std::setw(11) << g_lockfree_result.lookup_time_ms << " | "
            << std::setw(11) << g_lockfree_result.delete_time_ms << " | "
            << std::setw(11) << g_lockfree_result.total_time_ms << " |\n";

        std::cout << "  | " << std::left << std::setw(19) << g_rbtree_result.name << " | "
            << std::right << std::setw(11) << std::fixed << std::setprecision(3) << g_rbtree_result.insert_time_ms << " | "
            << std::setw(11) << g_rbtree_result.lookup_time_ms << " | "
            << std::setw(11) << g_rbtree_result.delete_time_ms << " | "
            << std::setw(11) << g_rbtree_result.total_time_ms << " |\n";

        std::cout << "  +---------------------+-------------+-------------+-------------+-------------+\n";

        // 2. АНАЛИЗ РЕЗУЛЬТАТОВ
        std::cout << "\n  Анализ результатов:\n";

        // Лучшая по вставке
        double min_insert = my_min(g_hash_result.insert_time_ms,
            g_lockfree_result.insert_time_ms,
            g_rbtree_result.insert_time_ms);
        std::string best_insert = (min_insert == g_hash_result.insert_time_ms) ? "Хеш-таблица" :
            (min_insert == g_lockfree_result.insert_time_ms) ? "Неблокирующая HT" :
            "Красно-чёрное дерево";
        std::cout << "  - Лучшая по вставке: " << best_insert
            << " (" << std::fixed << std::setprecision(3) << min_insert << " мс)\n";

        // Лучшая по поиску
        double min_lookup = my_min(g_hash_result.lookup_time_ms,
            g_lockfree_result.lookup_time_ms,
            g_rbtree_result.lookup_time_ms);
        std::string best_lookup = (min_lookup == g_hash_result.lookup_time_ms) ? "Хеш-таблица" :
            (min_lookup == g_lockfree_result.lookup_time_ms) ? "Неблокирующая HT" :
            "Красно-чёрное дерево";
        std::cout << "  - Лучшая по поиску: " << best_lookup
            << " (" << std::fixed << std::setprecision(3) << min_lookup << " мс)\n";

        // Лучшая по удалению
        double min_delete = my_min(g_hash_result.delete_time_ms,
            g_lockfree_result.delete_time_ms,
            g_rbtree_result.delete_time_ms);
        std::string best_delete = (min_delete == g_hash_result.delete_time_ms) ? "Хеш-таблица" :
            (min_delete == g_lockfree_result.delete_time_ms) ? "Неблокирующая HT" :
            "Красно-чёрное дерево";
        std::cout << "  - Лучшая по удалению: " << best_delete
            << " (" << std::fixed << std::setprecision(3) << min_delete << " мс)\n";

        // Лучшая по общему времени
        double min_total = my_min(g_hash_result.total_time_ms,
            g_lockfree_result.total_time_ms,
            g_rbtree_result.total_time_ms);
        std::string best_total = (min_total == g_hash_result.total_time_ms) ? "Хеш-таблица" :
            (min_total == g_lockfree_result.total_time_ms) ? "Неблокирующая HT" :
            "Красно-чёрное дерево";
        std::cout << "  - Лучшая по общему времени: " << best_total
            << " (" << std::fixed << std::setprecision(3) << min_total << " мс)\n";

        // 3. СРАВНЕНИЕ С ТЕОРЕТИЧЕСКОЙ СЛОЖНОСТЬЮ
        std::cout << "\n  Сравнение с теоретической сложностью:\n";
        std::cout << "  +------------------------+------------------+------------------+\n";
        std::cout << "  | Структура              | Теоретическая    | Реальная (мкс/эл)|\n";
        std::cout << "  +------------------------+------------------+------------------+\n";

        double hash_us = g_hash_result.total_time_ms / 10000 * 1000;
        double lockfree_us = g_lockfree_result.total_time_ms / 10000 * 1000;
        double rbtree_us = g_rbtree_result.total_time_ms / 10000 * 1000;

        std::cout << "  | " << std::left << std::setw(22) << "Хеш-таблица" << " | "
            << std::left << std::setw(16) << "O(1) сред." << " | "
            << std::right << std::setw(16) << std::fixed << std::setprecision(3) << hash_us << " |\n";

        std::cout << "  | " << std::left << std::setw(22) << "Неблокирующая HT" << " | "
            << std::left << std::setw(16) << "O(1) сред." << " | "
            << std::right << std::setw(16) << std::fixed << std::setprecision(3) << lockfree_us << " |\n";

        std::cout << "  | " << std::left << std::setw(22) << "Красно-чёрное дерево" << " | "
            << std::left << std::setw(16) << "O(log n)" << " | "
            << std::right << std::setw(16) << std::fixed << std::setprecision(3) << rbtree_us << " |\n";

        std::cout << "  +------------------------+------------------+------------------+\n";

        // 4. ВЫВОДЫ ПО СТРУКТУРАМ ДАННЫХ
        std::cout << "\n  Выводы по хеш-таблицам и кчд:\n";

        // Сравнение хеш-таблицы и КЧД
        double ratio = 0;
        std::string faster, slower;
        if (g_hash_result.total_time_ms < g_rbtree_result.total_time_ms) {
            ratio = g_rbtree_result.total_time_ms / g_hash_result.total_time_ms;
            faster = "Хеш-таблица";
            slower = "Красно-чёрное дерево";
            std::cout << "  - " << faster << " быстрее " << slower
                << " в " << std::fixed << std::setprecision(2) << ratio << " раза\n";
        }
        else {
            ratio = g_hash_result.total_time_ms / g_rbtree_result.total_time_ms;
            faster = "Красно-чёрное дерево";
            slower = "Хеш-таблица";
            std::cout << "  - " << faster << " быстрее " << slower
                << " в " << std::fixed << std::setprecision(2) << ratio << " раза\n";
        }

        // Сравнение блокирующей и неблокирующей версий
        if (g_hash_result.total_time_ms < g_lockfree_result.total_time_ms) {
            ratio = g_lockfree_result.total_time_ms / g_hash_result.total_time_ms;
            std::cout << "  - Блокирующая хеш-таблица быстрее неблокирующей в "
                << std::fixed << std::setprecision(2) << ratio << " раза\n";
        }
        else {
            ratio = g_hash_result.total_time_ms / g_lockfree_result.total_time_ms;
            std::cout << "  - Неблокирующая хеш-таблица быстрее блокирующей в "
                << std::fixed << std::setprecision(2) << ratio << " раза\n";
        }

        // Общий вывод
        if (g_hash_result.total_time_ms < g_rbtree_result.total_time_ms) {
            std::cout << "\n  Для 10000 элементов Хеш-таблица показывает лучшую общую\n";
            std::cout << "  производительность благодаря O(1) амортизированной сложности.\n";
        }
        else {
            std::cout << "\n  Для 10000 элементов Красно-чёрное дерево показывает лучшую\n";
            std::cout << "  общую производительность благодаря малым скрытым константам.\n";
            std::cout << "  При увеличении размера данных преимущество хеш-таблицы должно\n";
            std::cout << "  стать заметнее.\n";
        }

        // 5. СРАВНЕНИЕ АЛГОРИТМОВ СОРТИРОВКИ
        std::cout << "\n  2. АЛГОРИТМЫ СОРТИРОВКИ (сравнение для 10000 элементов):\n";
        std::cout << "  +---------------------+-------------+-----------------+\n";
        std::cout << "  | Алгоритм            | Время (мс)  | мкс/элемент     |\n";
        std::cout << "  +---------------------+-------------+-----------------+\n";
        std::cout << "  | " << std::left << std::setw(19) << g_quick_sort_result.name << " | "
            << std::right << std::setw(11) << std::fixed << std::setprecision(3) << g_quick_sort_result.time_ms << " | "
            << std::setw(15) << std::fixed << std::setprecision(3) << g_quick_sort_result.us_per_element << " |\n";
        std::cout << "  | " << std::left << std::setw(19) << g_insertion_sort_result.name << " | "
            << std::right << std::setw(11) << std::fixed << std::setprecision(3) << g_insertion_sort_result.time_ms << " | "
            << std::setw(15) << std::fixed << std::setprecision(3) << g_insertion_sort_result.us_per_element << " |\n";
        std::cout << "  +---------------------+-------------+-----------------+\n";

        // Анализ сортировок
        if (g_quick_sort_result.time_ms > 0 && g_insertion_sort_result.time_ms > 0) {
            std::cout << "\n  Вывод по сортировкам:\n";
            double speedup = g_insertion_sort_result.time_ms / g_quick_sort_result.time_ms;
            std::cout << "  Быстрая сортировка быстрее вставок в "
                << std::fixed << std::setprecision(2) << speedup << " раз\n";
            std::cout << "\n  Для 10000 элементов:\n";
            std::cout << "  - Хеш-таблица: лучший поиск (" << g_hash_result.lookup_time_ms << " мс)\n";
            std::cout << "  - КЧД: лучшая вставка (" << g_rbtree_result.insert_time_ms << " мс)\n";
            std::cout << "  - Быстрая сортировка: O(n log n) ~ "
                << g_quick_sort_result.us_per_element << " мкс/элемент\n";
            std::cout << "  - Сортировка вставками: O(n^2) ~ "
                << g_insertion_sort_result.us_per_element << " мкс/элемент\n";
        }

        // 6. РЕКОМЕНДАЦИИ
        std::cout << "\n  Рекомендации:\n";
        std::cout << "  - Для операций поиска: Хеш-таблица (O(1))\n";
        std::cout << "  - Для упорядоченных данных: Красно-чёрное дерево (O(log n))\n";
        std::cout << "  - Для многопоточных приложений: Неблокирующая хеш-таблица\n";
        std::cout << "  - Для сортировки больших массивов: Быстрая сортировка\n";
        std::cout << "  - Для маленьких массивов (< 100): Сортировка вставками\n";
    }

    std::cout << "\n\n  Нажмите Enter для выхода...";
    std::cin.get();
    return 0;
}