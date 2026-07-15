#ifndef HASH_TABLE_TASK2_H
#define HASH_TABLE_TASK2_H

#include <stdbool.h>  // Для использования типа bool

// Узел цепочки (для метода цепочек) - используется при коллизиях
typedef struct HashNode {
    char* key;              // Ключ (строка) - идентификатор элемента
    int value;              // Значение (для простоты используем int)
    struct HashNode* next;  // Указатель на следующий элемент в цепочке (при коллизиях)
} HashNode;

// Структура хеш-таблицы
typedef struct {
    HashNode** buckets;     // Массив указателей на головы цепочек (бакеты)
    int capacity;           // Размер массива (всегда степень двойки для оптимизации)
    int size;               // Количество хранимых пар (ключ, значение)
    int threshold;          // Порог для перехеширования (capacity * 0.75)
    int collisions;         // Счётчик коллизий (для статистики)
} HashTable;

// Обёртка для C++ - позволяет вызывать C-функции из C++ кода
#ifdef __cplusplus
extern "C" {
#endif

    // Создание и уничтожение
    HashTable* create_hash_table(int initial_capacity);  // Выделяет память под таблицу
    void free_hash_table(HashTable* ht);                 // Освобождает память

    // Основные операции
    void hash_table_insert(HashTable** ht_ptr, const char* key, int value);  // Вставка пары
    bool hash_table_get(HashTable* ht, const char* key, int* out_value);     // Поиск по ключу
    bool hash_table_remove(HashTable* ht, const char* key);                  // Удаление по ключу
    bool hash_table_contains(HashTable* ht, const char* key);                // Проверка наличия

    // Вспомогательные
    int hash_table_size(HashTable* ht);                                      // Текущий размер
    void hash_table_stats(HashTable* ht, int* out_collisions, float* out_avg_chain); // Статистика
    void print_hash_table(HashTable* ht);                                    // Печать содержимого

    // Хеш-функция - преобразует строку в индекс бакета
    unsigned int hash_function(const char* key, int capacity);

#ifdef __cplusplus
}
#endif

#endif // HASH_TABLE_TASK2_H