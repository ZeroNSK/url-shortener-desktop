#pragma once
#include <string>
using std::string;


class QueueNode {
public:
    string value;        
    QueueNode* next;
    
    QueueNode(const string& val) : value(val), next(nullptr) {}
};

class Queue {
private:
    QueueNode* head;    // первый элемент
    QueueNode* tail;    // последний элемент
    
    void copyFrom(const Queue& other);

public:
    Queue();
    ~Queue();
    Queue(const Queue& other);
    Queue& operator=(const Queue& other);
    
    // Операции
    void push(const string& value);
    bool pop(string& outValue);
    bool empty() const;
    void clear();
    
    friend struct Database;
};