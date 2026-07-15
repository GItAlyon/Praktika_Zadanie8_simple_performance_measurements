#include "rbtree_task5.h"
#include <iostream>
#include <string>

// Реализация красно-черного дерева

// Конструктор узла
template<typename Key, typename Value>
RedBlackTree<Key, Value>::Node::Node(const Key& k, const Value& v, Color c)
    : key(k), value(v), color(c), left(nullptr), right(nullptr), parent(nullptr) {}

// Конструктор дерева - создаёт пустое дерево с фиктивным листом nil
template<typename Key, typename Value>
RedBlackTree<Key, Value>::RedBlackTree() {
    nil = new Node(Key(), Value(), BLACK);  // nil всегда чёрный
    nil->left = nil->right = nil;
    nil->parent = nil;
    root = nil;  // Дерево пустое
}

// Деструктор - рекурсивно удаляет все узлы
template<typename Key, typename Value>
RedBlackTree<Key, Value>::~RedBlackTree() {
    clear(root);
    delete nil;
}

// Поиск узла с минимальным ключом в поддереве
template<typename Key, typename Value>
typename RedBlackTree<Key, Value>::Node*
RedBlackTree<Key, Value>::minimum(Node* node) const {
    while (node->left != nil) {
        node = node->left;
    }
    return node;
}

// Левый поворот вокруг узла x
template<typename Key, typename Value>
void RedBlackTree<Key, Value>::leftRotate(Node* x) {
    Node* y = x->right;          // y - правый потомок x
    x->right = y->left;          // Поддерево b становится правым потомком x
    if (y->left != nil) {
        y->left->parent = x;      // Обновляем родителя для b
    }
    y->parent = x->parent;        // Поднимаем y на место x
    if (x->parent == nil) {
        root = y;                 // Если x был корнем, y становится корнем
    }
    else if (x == x->parent->left) {
        x->parent->left = y;      // y становится левым потомком родителя x
    }
    else {
        x->parent->right = y;     // y становится правым потомком родителя x
    }
    y->left = x;                  // x становится левым потомком y
    x->parent = y;               // Обновляем родителя x
}

// Правый поворот вокруг узла y (симметричен левому повороту)
template<typename Key, typename Value>
void RedBlackTree<Key, Value>::rightRotate(Node* y) {
    Node* x = y->left;           // x - левый потомок y
    y->left = x->right;          // Поддерево b становится левым потомком y
    if (x->right != nil) {
        x->right->parent = y;     // Обновляем родителя для b
    }
    x->parent = y->parent;        // Поднимаем x на место y
    if (y->parent == nil) {
        root = x;                 // Если y был корнем, x становится корнем
    }
    else if (y == y->parent->left) {
        y->parent->left = x;      // x становится левым потомком родителя y
    }
    else {
        y->parent->right = x;     // x становится правым потомком родителя y
    }
    x->right = y;                 // y становится правым потомком x
    y->parent = x;               // Обновляем родителя y
}

// Восстановление свойств красно-черного дерева после вставки
template<typename Key, typename Value>
void RedBlackTree<Key, Value>::fixInsert(Node* z) {
    // Пока родитель красный - нарушено свойство 4
    while (z->parent->color == RED) {
        if (z->parent == z->parent->parent->left) {
            Node* y = z->parent->parent->right;  // "дядя" z

            if (y->color == RED) {
                // Случай 1: дядя красный - перекрашиваем
                z->parent->color = BLACK;
                y->color = BLACK;
                z->parent->parent->color = RED;
                z = z->parent->parent;  // Поднимаемся к деду
            }
            else {
                // Случай 2: дядя чёрный
                if (z == z->parent->right) {
                    // Если z - правый потомок, делаем левый поворот
                    z = z->parent;
                    leftRotate(z);
                }
                // Случай 3: z - левый потомок
                z->parent->color = BLACK;
                z->parent->parent->color = RED;
                rightRotate(z->parent->parent);
            }
        }
        else {
            // Симметричный случай: родитель z - правый потомок деда
            Node* y = z->parent->parent->left;   // "дядя" z

            if (y->color == RED) {
                // Случай 1: дядя красный
                z->parent->color = BLACK;
                y->color = BLACK;
                z->parent->parent->color = RED;
                z = z->parent->parent;
            }
            else {
                // Случай 2: дядя чёрный
                if (z == z->parent->left) {
                    z = z->parent;
                    rightRotate(z);
                }
                // Случай 3: z - правый потомок
                z->parent->color = BLACK;
                z->parent->parent->color = RED;
                leftRotate(z->parent->parent);
            }
        }
    }
    root->color = BLACK;  // Корень всегда чёрный
}

// Внутренний метод вставки узла
template<typename Key, typename Value>
void RedBlackTree<Key, Value>::insert(Node* z) {
    Node* y = nil;    // Родитель вставляемого узла
    Node* x = root;   // Текущий узел для поиска места вставки

    // Поиск места для вставки (как в обычном BST)
    while (x != nil) {
        y = x;
        if (z->key < x->key) {
            x = x->left;
        }
        else if (z->key > x->key) {
            x = x->right;
        }
        else {
            // Ключ уже существует - обновляем значение
            x->value = z->value;
            delete z;
            return;
        }
    }

    // Вставляем новый узел
    z->parent = y;
    if (y == nil) {
        root = z;           // Дерево было пустым
    }
    else if (z->key < y->key) {
        y->left = z;        // Вставляем как левого потомка
    }
    else {
        y->right = z;       // Вставляем как правого потомка
    }

    z->left = nil;
    z->right = nil;
    z->color = RED;         // Новый узел всегда красный

    fixInsert(z);           // Восстанавливаем свойства
}

// Замена одного узла другим (поддеревьями)
template<typename Key, typename Value>
void RedBlackTree<Key, Value>::transplant(Node* u, Node* v) {
    if (u->parent == nil) {
        root = v;                 // u был корнем
    }
    else if (u == u->parent->left) {
        u->parent->left = v;      // u был левым потомком
    }
    else {
        u->parent->right = v;     // u был правым потомком
    }
    v->parent = u->parent;        // Обновляем родителя для v
}

// Восстановление свойств красно-черного дерева после удаления
template<typename Key, typename Value>
void RedBlackTree<Key, Value>::fixDelete(Node* x) {
    while (x != root && x->color == BLACK) {
        if (x == x->parent->left) {
            Node* w = x->parent->right;  // Брат x

            if (w->color == RED) {
                // Случай 1: брат красный
                w->color = BLACK;
                x->parent->color = RED;
                leftRotate(x->parent);
                w = x->parent->right;
            }

            if (w->left->color == BLACK && w->right->color == BLACK) {
                // Случай 2: оба ребенка брата черные
                w->color = RED;
                x = x->parent;  // Поднимаемся к родителю
            }
            else {
                if (w->right->color == BLACK) {
                    // Случай 3: правый ребенок брата черный
                    w->left->color = BLACK;
                    w->color = RED;
                    rightRotate(w);
                    w = x->parent->right;
                }
                // Случай 4: правый ребенок брата красный
                w->color = x->parent->color;
                x->parent->color = BLACK;
                w->right->color = BLACK;
                leftRotate(x->parent);
                x = root;  // Завершаем
            }
        }
        else {
            // Симметричный случай: x - правый потомок
            Node* w = x->parent->left;   // Брат x

            if (w->color == RED) {
                // Случай 1: брат красный
                w->color = BLACK;
                x->parent->color = RED;
                rightRotate(x->parent);
                w = x->parent->left;
            }

            if (w->right->color == BLACK && w->left->color == BLACK) {
                // Случай 2: оба ребенка брата черные
                w->color = RED;
                x = x->parent;
            }
            else {
                if (w->left->color == BLACK) {
                    // Случай 3: левый ребенок брата черный
                    w->right->color = BLACK;
                    w->color = RED;
                    leftRotate(w);
                    w = x->parent->left;
                }
                // Случай 4: левый ребенок брата красный
                w->color = x->parent->color;
                x->parent->color = BLACK;
                w->left->color = BLACK;
                rightRotate(x->parent);
                x = root;  // Завершаем
            }
        }
    }
    x->color = BLACK;  // Гарантируем, что x черный
}

// Внутренний метод удаления узла
template<typename Key, typename Value>
void RedBlackTree<Key, Value>::deleteNode(Node* z) {
    Node* y = z;                // y - узел, который будет удален
    Node* x;                    // x - ребенок, который заменит y
    Color yOriginalColor = y->color;  // Исходный цвет y

    if (z->left == nil) {
        x = z->right;
        transplant(z, z->right);  // Заменяем z на правого потомка
    }
    else if (z->right == nil) {
        x = z->left;
        transplant(z, z->left);   // Заменяем z на левого потомка
    }
    else {
        y = minimum(z->right);     // Находим минимальный узел в правом поддереве
        yOriginalColor = y->color;
        x = y->right;

        if (y->parent == z) {
            x->parent = y;        // y - прямой потомок z
        }
        else {
            transplant(y, y->right);  // Заменяем y на его правого потомка
            y->right = z->right;
            y->right->parent = y;
        }

        transplant(z, y);          // Заменяем z на y
        y->left = z->left;
        y->left->parent = y;
        y->color = z->color;       // y получает цвет z
    }

    delete z;  // Освобождаем память

    // Если удаленный узел был черным, нужно восстановить баланс
    if (yOriginalColor == BLACK) {
        fixDelete(x);
    }
}

// Поиск узла по ключу
template<typename Key, typename Value>
typename RedBlackTree<Key, Value>::Node*
RedBlackTree<Key, Value>::findNode(const Key& key) const {
    Node* current = root;
    while (current != nil) {
        if (key < current->key) {
            current = current->left;
        }
        else if (key > current->key) {
            current = current->right;
        }
        else {
            return current;  // Ключ найден
        }
    }
    return nullptr;  // Ключ не найден
}

// Симметричный обход дерева (in-order traversal)
template<typename Key, typename Value>
void RedBlackTree<Key, Value>::inorder(Node* node, std::vector<std::pair<Key, Value>>& result) const {
    if (node == nil) return;
    inorder(node->left, result);        // Сначала левое поддерево
    result.push_back({ node->key, node->value });  // Затем корень
    inorder(node->right, result);       // Затем правое поддерево
}

// Очистка дерева (рекурсивное удаление всех узлов)
template<typename Key, typename Value>
void RedBlackTree<Key, Value>::clear(Node* node) {
    if (node == nil) return;
    clear(node->left);
    clear(node->right);
    delete node;
}

// Вычисление чёрной высоты поддерева
template<typename Key, typename Value>
int RedBlackTree<Key, Value>::blackHeight(Node* node) const {
    if (node == nil) return 1;  // Лист считается черным
    int leftBH = blackHeight(node->left);
    int rightBH = blackHeight(node->right);
    if (leftBH != rightBH) return -1;  // Нарушено свойство 5
    return leftBH + (node->color == BLACK ? 1 : 0);
}

// Проверка свойств красно-черного дерева
template<typename Key, typename Value>
bool RedBlackTree<Key, Value>::validateProperties(Node* node) const {
    if (node == nil) return true;
    // Свойство 4: у красного узла оба ребенка черные
    if (node->color == RED && node->parent->color == RED) return false;
    // Рекурсивная проверка поддеревьев
    if (!validateProperties(node->left)) return false;
    if (!validateProperties(node->right)) return false;
    return true;
}

// Публичные методы

template<typename Key, typename Value>
void RedBlackTree<Key, Value>::insert(const Key& key, const Value& value) {
    Node* z = new Node(key, value);
    insert(z);
}

template<typename Key, typename Value>
bool RedBlackTree<Key, Value>::remove(const Key& key) {
    Node* z = findNode(key);
    if (z == nullptr) return false;
    deleteNode(z);
    return true;
}

template<typename Key, typename Value>
bool RedBlackTree<Key, Value>::find(const Key& key, Value& value) const {
    Node* z = findNode(key);
    if (z == nullptr) return false;
    value = z->value;
    return true;
}

template<typename Key, typename Value>
bool RedBlackTree<Key, Value>::contains(const Key& key) const {
    return findNode(key) != nullptr;
}

template<typename Key, typename Value>
std::vector<std::pair<Key, Value>> RedBlackTree<Key, Value>::toVector() const {
    std::vector<std::pair<Key, Value>> result;
    inorder(root, result);
    return result;
}

template<typename Key, typename Value>
bool RedBlackTree<Key, Value>::isEmpty() const {
    return root == nil;
}

template<typename Key, typename Value>
void RedBlackTree<Key, Value>::print() const {
    if (root == nil) {
        std::cout << "Дерево пусто" << std::endl;
        return;
    }
}

template<typename Key, typename Value>
bool RedBlackTree<Key, Value>::validate() const {
    if (root == nil) return true;
    if (root->color != BLACK) return false;  // Свойство 2: корень черный
    int bh = blackHeight(root);
    if (bh == -1) return false;  // Свойство 5: черная высота одинакова
    return validateProperties(root);  // Свойство 4: красный не имеет красного родителя
}

// Реализация AssociativeArray

template<typename Key, typename Value>
AssociativeArray<Key, Value>::AssociativeArray() : size_(0) {}

template<typename Key, typename Value>
void AssociativeArray<Key, Value>::insert(const Key& key, const Value& value) {
    if (!tree.contains(key)) {
        size_++;  // Новый ключ - увеличиваем размер
    }
    tree.insert(key, value);
}

template<typename Key, typename Value>
bool AssociativeArray<Key, Value>::find(const Key& key, Value& value) const {
    return tree.find(key, value);
}

template<typename Key, typename Value>
bool AssociativeArray<Key, Value>::contains(const Key& key) const {
    return tree.contains(key);
}

template<typename Key, typename Value>
bool AssociativeArray<Key, Value>::remove(const Key& key) {
    if (tree.remove(key)) {
        size_--;  // Успешное удаление - уменьшаем размер
        return true;
    }
    return false;
}

template<typename Key, typename Value>
std::vector<std::pair<Key, Value>> AssociativeArray<Key, Value>::toSortedVector() const {
    return tree.toVector();
}

template<typename Key, typename Value>
size_t AssociativeArray<Key, Value>::size() const {
    return size_;
}

template<typename Key, typename Value>
bool AssociativeArray<Key, Value>::empty() const {
    return size_ == 0;
}

template<typename Key, typename Value>
void AssociativeArray<Key, Value>::print() const {
    auto elements = tree.toVector();
    std::cout << "Ассоциативный массив (отсортировано по ключам):" << std::endl;
    for (const auto& p : elements) {
        std::cout << "  " << p.first << " -> " << p.second << std::endl;
    }
    std::cout << "Размер: " << size_ << std::endl;
}

// Явные инстанциации для часто используемых типов
template class RedBlackTree<int, int>;
template class RedBlackTree<std::string, int>;
template class AssociativeArray<int, int>;
template class AssociativeArray<std::string, int>;

// Функции для бенчмарка
extern "C" void run_rbtree_benchmarks_task5() {
    int sizes[] = { 1000, 5000, 10000 };
    for (int i = 0; i < 3; i++) {
        benchmark_rbtree_task5_int(sizes[i]);
        if (i < 2) printf("\n");
    }
}