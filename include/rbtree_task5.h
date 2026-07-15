#ifndef RBTREE_TASK5_H
#define RBTREE_TASK5_H

#include <vector>    // Для возврата отсортированных элементов
#include <utility>   // Для std::pair

// Класс красно-черного дерева - самобалансирующегося двоичного дерева поиска
// Свойства красно-черного дерева:
// 1. Каждый узел красный или черный
// 2. Корень черный
// 3. Все листья (nil) черные
// 4. У красного узла оба ребенка черные
// 5. Все пути от узла до листьев содержат одинаковое количество черных узлов
template<typename Key, typename Value>
class RedBlackTree {
private:
    enum Color { RED, BLACK };  // Цвета узлов

    // Структура узла дерева
    struct Node {
        Key key;          // Ключ узла
        Value value;      // Значение узла
        Color color;      // Цвет узла (RED или BLACK)
        Node* left;       // Указатель на левого потомка
        Node* right;      // Указатель на правого потомка
        Node* parent;     // Указатель на родителя

        Node(const Key& k, const Value& v, Color c = RED);  // Конструктор
    };

    Node* root;    // Корень дерева
    Node* nil;     // Фиктивный лист (всегда черный) - упрощает реализацию

    // Вспомогательные методы для балансировки
    Node* minimum(Node* node) const;           // Поиск минимального элемента в поддереве
    void leftRotate(Node* x);                  // Левый поворот вокруг узла x
    void rightRotate(Node* y);                 // Правый поворот вокруг узла y
    void fixInsert(Node* z);                   // Восстановление свойств после вставки
    void insert(Node* z);                      // Внутренняя вставка узла
    void transplant(Node* u, Node* v);         // Замена поддерева u на v
    void fixDelete(Node* x);                   // Восстановление свойств после удаления
    void deleteNode(Node* z);                  // Внутреннее удаление узла
    Node* findNode(const Key& key) const;      // Поиск узла по ключу
    void inorder(Node* node, std::vector<std::pair<Key, Value>>& result) const; // Симметричный обход
    void clear(Node* node);                    // Рекурсивное удаление всех узлов
    int blackHeight(Node* node) const;         // Вычисление чёрной высоты
    bool validateProperties(Node* node) const; // Проверка свойств красно-черного дерева

public:
    RedBlackTree();   // Конструктор - создаёт пустое дерево
    ~RedBlackTree();  // Деструктор - очищает память

    // Основные операции
    void insert(const Key& key, const Value& value);          // Вставка (O(log n))
    bool remove(const Key& key);                               // Удаление (O(log n))
    bool find(const Key& key, Value& value) const;            // Поиск (O(log n))
    bool contains(const Key& key) const;                      // Проверка наличия (O(log n))
    std::vector<std::pair<Key, Value>> toVector() const;      // Все элементы в отсортированном порядке
    bool isEmpty() const;                                     // Проверка на пустоту
    void print() const;                                       // Визуализация дерева
    bool validate() const;                                    // Проверка всех свойств
};

// Ассоциативный массив (словарь) на основе красно-черного дерева
// Хранит пары (ключ, значение) с быстрым доступом по ключу
// Элементы всегда отсортированы по ключу
template<typename Key, typename Value>
class AssociativeArray {
private:
    RedBlackTree<Key, Value> tree;  // Базовое красно-черное дерево
    size_t size_;                    // Текущий размер словаря

public:
    AssociativeArray();  // Конструктор - создаёт пустой словарь

    void insert(const Key& key, const Value& value);          // Вставка или обновление
    bool find(const Key& key, Value& value) const;            // Поиск значения по ключу
    bool contains(const Key& key) const;                      // Проверка наличия ключа
    bool remove(const Key& key);                               // Удаление по ключу
    std::vector<std::pair<Key, Value>> toSortedVector() const; // Все элементы (отсортированы)
    size_t size() const;                                      // Текущий размер
    bool empty() const;                                       // Проверка на пустоту
    void print() const;                                       // Печать содержимого
};

// Функции-обёртки для C вызовов
#ifdef __cplusplus
extern "C" {
#endif

    void benchmark_rbtree_task5_int(int n);    // Бенчмарк для n элементов
    void run_rbtree_benchmarks_task5();         // Запуск серии бенчмарков

#ifdef __cplusplus
}
#endif

#endif // RBTREE_TASK5_H