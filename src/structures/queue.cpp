#include "queue.h"
#include <iostream>

Queue::Queue() {
    head = nullptr;      // голова отсутствует
    tail = nullptr;      // хвост тоже отсутствует
}

Queue::~Queue() {
    clear();
}

void Queue::copyFrom(const Queue& other) {
    // копируем элементы по очереди
    QueueNode* cur = other.head;
    while (cur) {
        push(cur->value);
        cur = cur->next;
    }
}

Queue::Queue(const Queue& other) {
    head = nullptr; // голова отсутствует
    tail = nullptr; // хвост тоже отсутствует
    copyFrom(other);
}

Queue& Queue::operator=(const Queue& other) {
    if (this != &other) { // проверяем самоприсваивание
        clear();
        copyFrom(other);
    }
    return *this;
}

bool Queue::empty() const {
    return head == nullptr;
}

void Queue::push(const string& value) {

    // создаём узел и инициализируем данные
    QueueNode* node = new QueueNode(value);

    // если очередь пустая — новый элемент и голова и хвост
    if (tail == nullptr) {
        head = node;
        tail = node;
        return;
    }

    // иначе — цепляем в конец
    tail->next = node;
    tail = node;
}

bool Queue::pop(string& outValue) {

    if (head == nullptr) // очередь пустая
        return false;

    QueueNode* node = head; 
    outValue = node->value; // данные для вывода

    head = node->next; // двигаем голову вперёд

    if (head == nullptr)// если  очередь пустая, то хвост тоже сбрасываем
        tail = nullptr;  

    delete node;                  
    return true;                
}

void Queue::clear() {

    QueueNode* cur = head;

    while (cur) {
        QueueNode* next = cur->next;
        delete cur;
        cur = next;
    }

    head = nullptr;
    tail = nullptr;
}