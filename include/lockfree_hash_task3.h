#ifndef LOCKFREE_HASH_TASK3_H
#define LOCKFREE_HASH_TASK3_H

#include <atomic>    // Для атомарных операций
#include <cstdint>   // Для uint32_t
#include <cstddef>   // Для size_t

// Структура узла списка для неблокирующей хеш-таблицы
struct NodeLF {
    char* key;                      // Ключ (строка)
    void* value;                    // Значение (указатель на произвольные данные)
    std::atomic<bool> deleted;      // Флаг логического удаления (атомарный)
    std::atomic<NodeLF*> next;      // Атомарный указатель на следующий узел

    NodeLF(const char* k, void* v); // Конструктор
    ~NodeLF();                      // Деструктор
};

// Упакованный указатель с тегом версии (для защиты от ABA-проблемы)
struct TaggedPtr {
    NodeLF* ptr;      // Указатель на узел
    uint32_t tag;     // Счётчик изменений (увеличивается при каждой модификации)
};

// Основная структура неблокирующей хеш-таблицы
class HashTableLockFree {
private:
    std::atomic<TaggedPtr> buckets[1024];  // Массив атомарных слотов (фиксированный размер)

    // Вспомогательные методы
    unsigned long hash_function(const char* str) const;  // Хеш-функция
    NodeLF* create_node(const char* key, void* value);   // Создание узла
    void destroy_node(NodeLF* node);                     // Уничтожение узла

public:
    HashTableLockFree();   // Конструктор - инициализирует таблицу
    ~HashTableLockFree();  // Деструктор - освобождает память

    // Запрещаем копирование (неблокирующие структуры сложно копировать)
    HashTableLockFree(const HashTableLockFree&) = delete;
    HashTableLockFree& operator=(const HashTableLockFree&) = delete;

    // Основные операции (все потокобезопасные)
    bool insert(const char* key, void* value);        // Вставка (CAS-цикл)
    void* lookup(const char* key);                    // Поиск (wait-free)
    bool remove(const char* key);                     // Логическое удаление (CAS)
    size_t size() const;                              // Количество активных элементов
    size_t size_logical() const;                      // Количество всех узлов (включая удалённые)
    void cleanup();                                   // Физическое удаление мёртвых узлов

    // Подсчёт заполненных бакетов (для статистики)
    size_t count_non_empty_buckets() const;
};

// Функции-обёртки для C вызовов (нужны для вызова из C-кода бенчмарка)
#ifdef __cplusplus
extern "C" {
#endif

    void benchmark_lockfree_hash_task3_int(int n);    // Запуск бенчмарка для n элементов
    void run_lockfree_benchmarks_task3();              // Запуск серии бенчмарков

#ifdef __cplusplus
}
#endif

#endif // LOCKFREE_HASH_TASK3_H