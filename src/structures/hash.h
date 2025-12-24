#pragma once
#include <string>
#include "../config/cfg.h"

using std::string;

//ключ, значение, ссылка на некст
class HashNode {
public:
    string key;
    string value;
    HashNode* next;
    
    HashNode(const string& k, const string& v, HashNode* nxt = nullptr) 
        : key(k), value(v), next(nxt) {}
};

class Hash {
private:
    HashNode* buckets[HASH_BUCKETS];
    
    static unsigned int hashStr(const string& s);
    void copyFrom(const Hash& other);

public:
    Hash();
    ~Hash();
    Hash(const Hash& other);
    Hash& operator=(const Hash& other);
    
    void set(const string& key, const string& value);
    bool get(const string& key, string& outValue) const;
    bool del(const string& key);
    
    bool contains(const string& key) const;
    
    void clear();
    
    friend void Hash_init(Hash* h);
    friend void HSET(Hash* h, const string& key, const string& value);
    friend bool HGET(Hash* h, const string& key, string& outValue);
    friend bool HDEL(Hash* h, const string& key);
    friend bool Hash_contains(Hash* h, const string& key);
    friend void Hash_clear(Hash* h);
    
    // Для доступа к buckets при сохранении/загрузке
    friend struct Database;
};