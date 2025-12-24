#include "stack.h"
#include <iostream>
#include <vector>

Stack::Stack() {
    top = nullptr;         
}

Stack::~Stack() {
    clear();
}

void Stack::copyFrom(const Stack& other) {
    // собираем элементы в массив для сохранения порядка
    std::vector<string> values;
    StackNode* cur = other.top;
    while (cur) {
        values.push_back(cur->value);
        cur = cur->next;
    }
    // добавляем в обратном порядке
    for (int i = values.size() - 1; i >= 0; --i) {
        push(values[i]);
    }
}

Stack::Stack(const Stack& other) {
    top = nullptr; // инициализируем пустым
    copyFrom(other);
}

Stack& Stack::operator=(const Stack& other) {
    if (this != &other) { // проверяем самоприсваивание
        clear();
        copyFrom(other);
    }
    return *this;
}

bool Stack::empty() const {
    return top == nullptr;// true если стек пуст
}

void Stack::push(const string& value) {

    // создаём новый узел стека
    StackNode* node = new StackNode(value, top);// новый узел указывает на старый первый элемент

    top = node;// теперь вершина указывает на new node
}
// бинарное дерево, вывести где находится слева/справа и вывести под каким номером находится

bool Stack::pop(string& outValue) {

    if (top == nullptr) // если стек пуст
        return false;

    StackNode* node = top;  // сохраняем старую вершину
    outValue = node->value; // копируем значение 

    top = node->next;
    delete node;                 

    return true;              
}

void Stack::clear() {

    StackNode* cur = top;

    while (cur) {
        StackNode* next = cur->next;
        delete cur;
        cur = next;
    }

    top = nullptr;  
}