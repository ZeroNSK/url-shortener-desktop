/**
 * РЕАЛИЗАЦИЯ МНОЖЕСТВА (SET) НА ОСНОВЕ ХЕШ-ТАБЛИЦЫ
 * 
 * Множество хранит уникальные строковые значения с быстрым доступом O(1) в среднем.
 * Использует хеширование с цепочками для разрешения коллизий.
 * 
 * Основные операции:
 * - add() - добавление элемента (игнорирует дубликаты)
 * - remove() - удаление элемента
 * - isMember() - проверка наличия элемента
 * - clear() - очистка всего множества
 */

#include "set.h"
#include <cstring>
#include <iostream>

/**
 * Хеш-функция для строк (алгоритм djb2)
 * 
 * @param s - строка для хеширования
 * @return индекс корзины в диапазоне [0, SET_BUCKETS-1]
 * 
 * Использует простой но эффективный алгоритм:
 * hash = hash * 131 + символ
 */
unsigned int Set::hashStr(const string& s) {
    unsigned int h = 0; // Начальное значение хеша
    for (char c : s) {  // Проходим по каждому символу
        h = h * 131 + (unsigned char)c; // Умножаем на простое число и добавляем символ
    }
    return h % SET_BUCKETS;  // Приводим к диапазону корзин
}

/**
 * Конструктор по умолчанию
 * Инициализирует пустое множество - все корзины указывают на nullptr
 */
Set::Set() {
    // Обнуляем все указатели на начала цепочек
    for (int i = 0; i < SET_BUCKETS; i++)
        buckets[i] = nullptr;
}

/**
 * Деструктор - освобождает всю память, занятую узлами
 */
Set::~Set() {
    clear(); // Удаляем все элементы и освобождаем память
}

/**
 * Копирование всех элементов из другого множества
 * 
 * @param other - множество-источник для копирования
 * Используется в конструкторе копирования и операторе присваивания
 */
void Set::copyFrom(const Set& other) {
    // Проходим по всем корзинам исходного множества
    for (int i = 0; i < SET_BUCKETS; i++) {
        SetNode* cur = other.buckets[i];
        // Проходим по цепочке в каждой корзине
        while (cur) {
            add(cur->value); // Добавляем элемент в текущее множество
            cur = cur->next;
        }
    }
}

Set::Set(const Set& other) {
    // обнуляем все указатели, чтобы список был пуст
    for (int i = 0; i < SET_BUCKETS; i++)
        buckets[i] = nullptr;
    copyFrom(other);
}

Set& Set::operator=(const Set& other) {
    if (this != &other) { // проверяем самоприсваивание
        clear();
        copyFrom(other);
    }
    return *this;
}

bool Set::isMember(const string& value) const {
    unsigned int h = hashStr(value); // вычисляем индекс корзины
    SetNode* cur = buckets[h]; // берём начало цепочки в этой корзине
    while (cur) {
        if (cur->value == value) return true;
        cur = cur->next;
    }
    return false;
}

bool Set::add(const string& value) {

    if (isMember(value)) return false; // если элемент уже есть - не добавляем

    unsigned int h = hashStr(value);
       
    // создаём новый узел: value + указатель на текущее начало цепочки
    SetNode* node = new SetNode(value, buckets[h]); 
    
    buckets[h] = node;

    return true;
}

bool Set::remove(const string& value) {
    unsigned int h = hashStr(value);
    SetNode* cur = buckets[h]; // текущий узел
    SetNode* prev = nullptr; // предыдущий для удаления

    while (cur) {
        if (cur->value == value) {
            if (prev) prev->next = cur->next; // если элемент не первый, то пропускаем удаляемый узел
            else      buckets[h] = cur->next; // если первый узел — смещаем голову
            delete cur;
            return true;
        }
        prev = cur;
        cur = cur->next;
    }
    return false;
}

void Set::clear() {
    for (int i = 0; i < SET_BUCKETS; i++) {
        SetNode* cur = buckets[i]; 
        while (cur) {
            SetNode* tmp = cur; // пока цепочка не пустая - сохраняем текущий
            cur = cur->next;
            delete tmp;  // удаляем текущий узел
        }
        buckets[i] = nullptr;
    }
}