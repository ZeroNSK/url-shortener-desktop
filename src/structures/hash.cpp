#include "hash.h"
#include <iostream>

unsigned int Hash::hashStr(const string& s) { 
    unsigned int h = 0;
    for (char c : s) // проходимся по всех элементам и применяем хешфункцию
        h = h * 131 + (unsigned char)c; 
    return h % 1024;   // индекс корзины
}

Hash::Hash() {
    for (int i = 0; i < 1024; i++) // проходимся по всему массиву корзин и зануляем 
        buckets[i] = nullptr;
}

Hash::~Hash() {
    clear();
}

void Hash::copyFrom(const Hash& other) {
    // копируем все пары ключ-значение из всех корзин
    for (int i = 0; i < HASH_BUCKETS; i++) {
        HashNode* cur = other.buckets[i];
        while (cur) {
            set(cur->key, cur->value);
            cur = cur->next;
        }
    }
}

Hash::Hash(const Hash& other) {
    for (int i = 0; i < 1024; i++) // проходимся по всему массиву корзин и зануляем 
        buckets[i] = nullptr;
    copyFrom(other);
}

Hash& Hash::operator=(const Hash& other) {
    if (this != &other) { // проверяем самоприсваивание
        clear();
        copyFrom(other);
    }
    return *this;
}

bool Hash::contains(const string& key) const {
    unsigned int idx = hashStr(key); // вычисляем индекс корзины
    HashNode* cur = buckets[idx]; // берем первый элемент в этой корзине

    while (cur) { 
        if (cur->key == key) // если нашли ключ
            return true;
        cur = cur->next; // идем дальше по цепочке
    }

    return false;
}

void Hash::set(const string& key, const string& value) {

    unsigned int idx = hashStr(key);
    HashNode* cur = buckets[idx]; // первый элемент в корзине

    // если нашли ключ — обновляем значение
    while (cur) {
        if (cur->key == key) {
            cur->value = value;
            return;
        }
        cur = cur->next;
    }

    // если не нашли ключ - создаём новый узел и вставляем в начало цепочки
    HashNode* node = new HashNode(key, value, buckets[idx]); // новый узел указывает на текущее начало цепочки
    buckets[idx] = node; // голова цепочки теперь новый узел
}

bool Hash::get(const string& key, string& outValue) const {

    unsigned int idx = hashStr(key); 
    HashNode* cur = buckets[idx];

    while (cur) {
        if (cur->key == key) {
            outValue = cur->value;
            return true;
        }
        cur = cur->next;
    }

    return false;
}

bool Hash::del(const string& key) {

    unsigned int idx = hashStr(key); // индекс корзины
    HashNode* cur = buckets[idx]; // текущий узел
    HashNode* prev = nullptr; // предыдущий узел для удаления

    while (cur) { 
        if (cur->key == key) {

            if (prev) // если не первый узел
                prev->next = cur->next; // пропускаем удаляемый узел
            else
                buckets[idx] = cur->next;   // удаление из головы

            delete cur;
            return true;
        }

        prev = cur;
        cur = cur->next;
    }
    
    return false;
}

void Hash::clear() {
    for (int i = 0; i < HASH_BUCKETS; i++) {
        HashNode* cur = buckets[i];
        while (cur) {
            HashNode* next = cur->next;
            delete cur;
            cur = next;
        }
        buckets[i] = nullptr;
    }
}