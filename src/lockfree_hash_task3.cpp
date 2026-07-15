#include "lockfree_hash_task3.h"
#include <cstring>
#include <cstdlib>
#include <cstdio>

// Конструктор узла - инициализирует ключ и значение
NodeLF::NodeLF(const char* k, void* v) {
    key = _strdup(k);  // Копируем строку
    value = v;
    deleted.store(false, std::memory_order_relaxed);  // Изначально не удалён
    next.store(nullptr, std::memory_order_relaxed);   // Следующего нет
}

// Деструктор узла - освобождает память ключа
NodeLF::~NodeLF() {
    free(key);
}

// Конструктор хеш-таблицы - инициализирует все бакеты пустыми
HashTableLockFree::HashTableLockFree() {
    for (int i = 0; i < 1024; i++) {
        TaggedPtr empty{ nullptr, 0 };  // Пустой указатель с тегом 0
        buckets[i].store(empty, std::memory_order_relaxed);
    }
}

// Деструктор - освобождает все узлы
HashTableLockFree::~HashTableLockFree() {
    for (int i = 0; i < 1024; i++) {
        TaggedPtr head = buckets[i].load(std::memory_order_acquire);
        NodeLF* curr = head.ptr;

        while (curr) {
            NodeLF* next = curr->next.load(std::memory_order_acquire);
            delete curr;
            curr = next;
        }
    }
}

// Хеш-функция djb2 для неблокирующей таблицы
unsigned long HashTableLockFree::hash_function(const char* str) const {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % 1024;  // 1024 бакета
}

// Создание узла с проверкой на ошибки
NodeLF* HashTableLockFree::create_node(const char* key, void* value) {
    NodeLF* node = new (std::nothrow) NodeLF(key, value);
    return node;
}

// Уничтожение узла
void HashTableLockFree::destroy_node(NodeLF* node) {
    delete node;
}

// Вставка элемента (Lock-Free с использованием CAS)
bool HashTableLockFree::insert(const char* key, void* value) {
    if (!key) return false;

    unsigned long bucket = hash_function(key);
    NodeLF* new_node = create_node(key, value);
    if (!new_node) return false;

    // CAS-цикл - пытаемся вставить, пока не получится
    while (true) {
        TaggedPtr old = buckets[bucket].load(std::memory_order_acquire);
        NodeLF* curr_head = old.ptr;

        // Проверяем, нет ли уже такого ключа (не удалённого)
        NodeLF* curr = curr_head;
        while (curr != nullptr) {
            if (!curr->deleted.load(std::memory_order_acquire) && strcmp(curr->key, key) == 0) {
                delete new_node;
                return false;  // Ключ уже существует
            }
            curr = curr->next.load(std::memory_order_acquire);
        }

        // Вставляем новый узел в голову списка
        new_node->next.store(curr_head, std::memory_order_relaxed);

        // Пытаемся атомарно заменить голову (CAS с тегом)
        TaggedPtr new_head{ new_node, old.tag + 1 };  // Увеличиваем тег

        if (buckets[bucket].compare_exchange_weak(old, new_head,
            std::memory_order_release, std::memory_order_relaxed)) {
            return true;  // Успешно вставлено
        }
        // CAS провалился - кто-то изменил голову
    }
}

// Поиск элемента (Wait-Free - без циклов)
void* HashTableLockFree::lookup(const char* key) {
    if (!key) return nullptr;

    unsigned long bucket = hash_function(key);
    TaggedPtr current = buckets[bucket].load(std::memory_order_acquire);
    NodeLF* curr = current.ptr;

    // Проходим по цепочке и ищем ключ
    while (curr != nullptr) {
        NodeLF* next = curr->next.load(std::memory_order_acquire);
        // Проверяем, что узел не удалён и ключ совпадает
        if (!curr->deleted.load(std::memory_order_acquire) && strcmp(curr->key, key) == 0) {
            return curr->value;  // Нашли
        }
        curr = next;
    }
    return nullptr;  // Не нашли
}

// Логическое удаление элемента (устанавливаем флаг deleted)
bool HashTableLockFree::remove(const char* key) {
    if (!key) return false;

    unsigned long bucket = hash_function(key);

    // Ищем узел с нужным ключом
    while (true) {
        TaggedPtr head = buckets[bucket].load(std::memory_order_acquire);
        NodeLF* curr = head.ptr;

        while (curr != nullptr) {
            NodeLF* next = curr->next.load(std::memory_order_acquire);

            // Нашли неудалённый узел с нужным ключом
            if (!curr->deleted.load(std::memory_order_acquire) && strcmp(curr->key, key) == 0) {
                // Пытаемся атомарно установить флаг deleted
                bool expected = false;
                if (curr->deleted.compare_exchange_weak(expected, true,
                    std::memory_order_release, std::memory_order_relaxed)) {
                    return true;  // Успешно логически удалили
                }
                // CAS провалился - кто-то другой удаляет этот узел
                break;
            }
            curr = next;
        }
        return false;  // Ключ не найден
    }
}

// Количество активных (не удалённых) элементов
size_t HashTableLockFree::size() const {
    size_t total = 0;

    for (int i = 0; i < 1024; i++) {
        TaggedPtr head = buckets[i].load(std::memory_order_acquire);
        NodeLF* curr = head.ptr;

        while (curr != nullptr) {
            if (!curr->deleted.load(std::memory_order_acquire)) {
                total++;
            }
            curr = curr->next.load(std::memory_order_acquire);
        }
    }
    return total;
}

// Количество всех узлов (включая удалённые)
size_t HashTableLockFree::size_logical() const {
    size_t total = 0;

    for (int i = 0; i < 1024; i++) {
        TaggedPtr head = buckets[i].load(std::memory_order_acquire);
        NodeLF* curr = head.ptr;

        while (curr != nullptr) {
            total++;
            curr = curr->next.load(std::memory_order_acquire);
        }
    }
    return total;
}

// Физическая очистка - удаление помеченных узлов
void HashTableLockFree::cleanup() {
    for (int bucket = 0; bucket < 1024; bucket++) {
        while (true) {
            TaggedPtr head = buckets[bucket].load(std::memory_order_acquire);
            NodeLF* curr = head.ptr;
            NodeLF* prev = nullptr;
            bool changed = false;

            // Проходим по списку и удаляем все помеченные узлы
            while (curr != nullptr) {
                NodeLF* next = curr->next.load(std::memory_order_acquire);

                if (curr->deleted.load(std::memory_order_acquire)) {
                    if (prev == nullptr) {
                        // Удаляем голову списка
                        TaggedPtr new_head{ next, head.tag + 1 };
                        if (buckets[bucket].compare_exchange_weak(head, new_head,
                            std::memory_order_release, std::memory_order_relaxed)) {
                            delete curr;
                            changed = true;
                            break;
                        }
                    }
                    else {
                        // Удаляем не голову
                        NodeLF* expected = curr;
                        if (prev->next.compare_exchange_weak(expected, next,
                            std::memory_order_release, std::memory_order_relaxed)) {
                            delete curr;
                            changed = true;
                            break;
                        }
                    }
                }

                prev = curr;
                curr = next;
            }

            if (!changed) break;  // Всё очищено
        }
    }
}

// Подсчёт заполненных бакетов (для статистики)
size_t HashTableLockFree::count_non_empty_buckets() const {
    size_t count = 0;
    for (int i = 0; i < 1024; i++) {
        TaggedPtr head = buckets[i].load(std::memory_order_acquire);
        NodeLF* curr = head.ptr;
        bool has_active = false;
        // Проверяем, есть ли в бакете активные (не удалённые) узлы
        while (curr != nullptr) {
            if (!curr->deleted.load(std::memory_order_acquire)) {
                has_active = true;
                break;
            }
            curr = curr->next.load(std::memory_order_acquire);
        }
        if (has_active) {
            count++;
        }
    }
    return count;
}

// C-функции для бенчмарка
extern "C" void run_lockfree_benchmarks_task3() {
    // Запуск бенчмарка с разными размерами
    int sizes[] = { 1000, 5000, 10000 };
    for (int i = 0; i < 3; i++) {
        benchmark_lockfree_hash_task3_int(sizes[i]);
        if (i < 2) printf("\n");
    }
}