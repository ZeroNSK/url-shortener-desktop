#pragma once
#include <string>
#include "../config/cfg.h"

using std::string;

class SetNode {
public:
    string value;
    SetNode* next;
    
    SetNode(const string& val, SetNode* nxt = nullptr) : value(val), next(nxt) {}
};

class Set {
private:
    SetNode* buckets[SET_BUCKETS];
    
    static unsigned int hashStr(const string& s);
    void copyFrom(const Set& other);

public:
    Set();
    ~Set();
    Set(const Set& other);
    Set& operator=(const Set& other);
    
    bool add(const string& value);
    bool remove(const string& value);
    bool isMember(const string& value) const;
    
    void clear();
    
    // Для доступа к buckets при сохранении/загрузке
    friend struct Database;
};