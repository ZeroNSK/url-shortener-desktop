#pragma once
#include <string>

using std::string;

class StackNode {
public:
    string value;  
    StackNode* next;
    
    StackNode(const string& val, StackNode* nxt = nullptr) : value(val), next(nxt) {}
};

class Stack {
private:
    StackNode* top;
    
    void copyFrom(const Stack& other);

public:
    Stack();
    ~Stack();
    Stack(const Stack& other);
    Stack& operator=(const Stack& other);
    
    void push(const string& value);
    bool pop(string& outValue);
    
    bool empty() const;
    
    void clear();
    
    // Для доступа к top при сохранении/загрузке
    friend struct Database;
};