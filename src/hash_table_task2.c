#define _CRT_SECURE_NO_WARNINGS
#include "hash_table_task2.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Возвращает ближайшую степень двойки, большую или равную x
// Используется для оптимизации: capacity всегда степень двойки
static int next_power_of_two(int x) {
    if (x <= 0) return 1;
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return x + 1;
}

// Хеш-функция djb2
// Хорошее распределение для строковых ключей
unsigned int hash_function(const char* key, int capacity) {
    unsigned long hash = 5381;  // Магическое начальное значение
    int c;

    // Основной цикл: hash = hash * 33 + c
    while ((c = *key++)) {
        hash = ((hash << 5) + hash) + c;  // hash * 33 + c
    }

    // Перемешивание для улучшения распределения
    hash ^= (hash >> 16);
    hash ^= (hash >> 8);
    hash ^= (hash >> 4);

    // Приведение к индексу в массиве (capacity - степень двойки)
    // Операция & быстрее, чем % для степени двойки
    return (unsigned int)(hash & (capacity - 1));
}

// Создание и уничтожение
HashTable* create_hash_table(int initial_capacity) {
    HashTable* ht = (HashTable*)malloc(sizeof(HashTable));
    if (!ht) {
        fprintf(stderr, "Ошибка выделения памяти для хеш-таблицы.\n");
        exit(EXIT_FAILURE);
    }

    // Приводим к степени двойки (минимум 8)
    ht->capacity = next_power_of_two(initial_capacity < 8 ? 8 : initial_capacity);
    ht->size = 0;
    ht->collisions = 0;
    ht->threshold = (int)(ht->capacity * 0.75f);  // Порог перехеширования

    // Выделяем память под бакеты (массив указателей)
    ht->buckets = (HashNode**)calloc(ht->capacity, sizeof(HashNode*));
    if (!ht->buckets) {
        fprintf(stderr, "Ошибка выделения памяти для бакетов.\n");
        free(ht);
        exit(EXIT_FAILURE);
    }

    return ht;
}

void free_hash_table(HashTable* ht) {
    if (!ht) return;

    // Освобождаем все цепочки
    for (int i = 0; i < ht->capacity; i++) {
        HashNode* current = ht->buckets[i];
        while (current) {
            HashNode* temp = current;
            current = current->next;
            free(temp->key);   // Освобождаем ключ
            free(temp);        // Освобождаем узел
        }
    }

    free(ht->buckets);  // Освобождаем массив бакетов
    free(ht);           // Освобождаем структуру
}

// Перехеширование - увеличивает ёмкость вдвое и перераспределяет все элементы
static void rehash(HashTable** ht_ptr) {
    HashTable* old_ht = *ht_ptr;
    HashTable* new_ht = create_hash_table(old_ht->capacity * 2);
    new_ht->size = 0;

    // Проходим по всем старым цепочкам
    for (int i = 0; i < old_ht->capacity; i++) {
        HashNode* current = old_ht->buckets[i];
        while (current) {
            // Вычисляем новый индекс для каждого элемента
            unsigned int index = hash_function(current->key, new_ht->capacity);
            // Создаём новый узел и вставляем в начало цепочки
            HashNode* new_node = (HashNode*)malloc(sizeof(HashNode));
            new_node->key = _strdup(current->key);
            new_node->value = current->value;
            new_node->next = new_ht->buckets[index];
            new_ht->buckets[index] = new_node;
            new_ht->size++;

            // Считаем коллизии в новой таблице
            if (new_node->next != NULL) {
                new_ht->collisions++;
            }

            current = current->next;
        }
    }

    // Заменяем старую таблицу новой
    free(old_ht->buckets);
    free(old_ht);
    *ht_ptr = new_ht;
}

// Вставка пары (ключ, значение)
void hash_table_insert(HashTable** ht_ptr, const char* key, int value) {
    HashTable* ht = *ht_ptr;

    // Проверяем необходимость перехеширования
    if (ht->size >= ht->threshold) {
        rehash(ht_ptr);
        ht = *ht_ptr;
    }

    unsigned int index = hash_function(key, ht->capacity);
    HashNode* current = ht->buckets[index];

    // Ищем ключ в цепочке (обновляем значение, если ключ уже есть)
    while (current) {
        if (strcmp(current->key, key) == 0) {
            current->value = value;
            return;
        }
        current = current->next;
    }

    // Ключ не найден - создаём новый узел в начале цепочки
    HashNode* new_node = (HashNode*)malloc(sizeof(HashNode));
    new_node->key = _strdup(key);
    new_node->value = value;
    new_node->next = ht->buckets[index];
    ht->buckets[index] = new_node;
    ht->size++;

    // Если в цепочке уже были элементы - это коллизия
    if (new_node->next != NULL) {
        ht->collisions++;
    }
}

// Поиск значения по ключу
bool hash_table_get(HashTable* ht, const char* key, int* out_value) {
    if (!ht) return false;

    unsigned int index = hash_function(key, ht->capacity);
    HashNode* current = ht->buckets[index];

    // Идём по цепочке и ищем ключ
    while (current) {
        if (strcmp(current->key, key) == 0) {
            if (out_value) *out_value = current->value;
            return true;
        }
        current = current->next;
    }
    return false;  // Ключ не найден
}

// Удаление пары по ключу
bool hash_table_remove(HashTable* ht, const char* key) {
    if (!ht) return false;

    unsigned int index = hash_function(key, ht->capacity);
    HashNode* current = ht->buckets[index];
    HashNode* prev = NULL;

    // Ищем ключ в цепочке
    while (current) {
        if (strcmp(current->key, key) == 0) {
            // Удаляем узел (переподвязываем цепочку)
            if (prev) {
                prev->next = current->next;
            }
            else {
                ht->buckets[index] = current->next;
            }
            free(current->key);
            free(current);
            ht->size--;
            return true;
        }
        prev = current;
        current = current->next;
    }
    return false;  // Ключ не найден
}

// Проверка наличия ключа
bool hash_table_contains(HashTable* ht, const char* key) {
    return hash_table_get(ht, key, NULL);
}

// Получение количества элементов
int hash_table_size(HashTable* ht) {
    return ht ? ht->size : 0;
}

// Получение статистики
void hash_table_stats(HashTable* ht, int* out_collisions, float* out_avg_chain) {
    if (!ht) {
        if (out_collisions) *out_collisions = 0;
        if (out_avg_chain) *out_avg_chain = 0.0f;
        return;
    }

    if (out_collisions) *out_collisions = ht->collisions;

    // Вычисляем среднюю длину цепочки (только для непустых бакетов)
    if (out_avg_chain) {
        int non_empty = 0;
        int total_items = 0;
        for (int i = 0; i < ht->capacity; i++) {
            if (ht->buckets[i] != NULL) {
                non_empty++;
                HashNode* current = ht->buckets[i];
                while (current) {
                    total_items++;
                    current = current->next;
                }
            }
        }
        *out_avg_chain = (non_empty > 0) ? (float)total_items / non_empty : 0.0f;
    }
}

// Печать содержимого хеш-таблицы (для отладки)
void print_hash_table(HashTable* ht) {
    if (!ht) {
        printf("Хеш-таблица пуста (NULL).\n");
        return;
    }
    printf("Хеш-таблица (размер: %d, ёмкость: %d, коллизий: %d):\n",
        ht->size, ht->capacity, ht->collisions);
    for (int i = 0; i < ht->capacity; i++) {
        if (ht->buckets[i] != NULL) {
            printf("  [%d] -> ", i);
            HashNode* current = ht->buckets[i];
            while (current) {
                printf("(%s: %d) ", current->key, current->value);
                if (current->next) printf("-> ");
                current = current->next;
            }
            printf("\n");
        }
    }
}