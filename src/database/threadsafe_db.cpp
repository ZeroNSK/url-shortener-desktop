#include "threadsafe_db.h"
#include <iostream>

using namespace std;


ThreadSafeDB::ThreadSafeDB() { // конструктор инициализирующий мьютексы
} 


bool ThreadSafeDB::load(const string& path) {
    lock_guard<mutex> file_lock(file_mutex);  // блок файловых операций
    lock_guard<mutex> db_lock(db_mutex);      // блок доступа к данным БД
    
    bool result = database.load(path);
    
    if (result) {
        last_save_path = path; // путь для автосейва
        Logger::getInstance().info("База данных загружена из " + path);
    } else {
        Logger::getInstance().logError("Не удалось загрузить базу данных из " + path, "ThreadSafeDB::load");
    }
    
    return result;
}

bool ThreadSafeDB::save(const string& path) {
    lock_guard<mutex> file_lock(file_mutex); 
    lock_guard<mutex> db_lock(db_mutex);    
    
    bool result = database.save(path);
    
    if (result) {
        last_save_path = path;
        Logger::getInstance().info("База данных успешно сохранена в " + path);
    } else {
        Logger::getInstance().logError("Не удалось сохранить базу данных в " + path, "ThreadSafeDB::save");
    }
    
    return result;
}

string ThreadSafeDB::executeCommand(const string& command) {
    istringstream ss(command);
    string cmd;
    ss >> cmd;
    
    string result;
    bool is_write_operation = false;
    
    // Блокируем доступ к данным БД для выполнения команды
    {
        lock_guard<mutex> db_lock(db_mutex);
        
        if (cmd == "SADD") {
            string name, value;
            ss >> name >> value;
            
            if (!database.sets.count(name)) {
                database.sets[name] = Set();
            }
            
            bool ok = database.sets[name].add(value);
            result = ok ? "1" : "0";
            is_write_operation = ok;
        }
        else if (cmd == "SREM") {
            string name, value;
            ss >> name >> value;
            
            if (database.sets.count(name)) {
                bool ok = database.sets[name].remove(value);
                result = ok ? "1" : "0"; 
                is_write_operation = ok;
            } else {
                result = "SET_NOT_EXISTS";
            }
        }
        else if (cmd == "SISMEMBER") {
            string name, value;
            ss >> name >> value;
            
            if (database.sets.count(name)) {
                bool exists = database.sets[name].isMember(value);
                result = exists ? "1" : "0"; 
            } else {
                result = "0";
            }
        }
        else if (cmd == "SPUSH") {
            string name, value;
            ss >> name >> value;
            
            if (!database.stacks.count(name)) {
                database.stacks[name] = Stack();
            }
            
            database.stacks[name].push(value);
            result = "OK";
            is_write_operation = true;
        }
        else if (cmd == "SPOP") {
            string name;
            ss >> name;
            
            if (database.stacks.count(name)) {
                string v;
                bool ok = database.stacks[name].pop(v);
                result = ok ? v : "EMPTY_STACK";
                is_write_operation = ok;
            } else {
                result = "STACK_NOT_EXISTS";
            }
        }
        else if (cmd == "QPUSH") {
            string name, value;
            ss >> name >> value;
            
            if (!database.queues.count(name)) {
                database.queues[name] = Queue();
            }
            
            database.queues[name].push(value);
            result = "OK"; 
            is_write_operation = true;
        }
        else if (cmd == "QPOP") {
            string name;
            ss >> name;
            
            if (database.queues.count(name)) {
                string v;
                bool ok = database.queues[name].pop(v);
                result = ok ? v : "EMPTY_QUEUE";
                is_write_operation = ok;
            } else {
                result = "QUEUE_NOT_EXISTS";
            }
        }
        else if (cmd == "HSET") {
            string name, key, value;
            ss >> name >> key >> value;
            
            if (!database.hashes.count(name)) {
                database.hashes[name] = Hash();
            }
            
            database.hashes[name].set(key, value);
            result = "OK"; 
            is_write_operation = true;
        }
        else if (cmd == "HDEL") {
            string name, key;
            ss >> name >> key;
            
            if (database.hashes.count(name)) {
                bool ok = database.hashes[name].del(key);
                result = ok ? "1" : "0";
                is_write_operation = ok;
            } else {
                result = "HASH_NOT_EXISTS";
            }
        }
        else if (cmd == "HGET") {
            string name, key;
            ss >> name >> key;
            
            if (database.hashes.count(name)) {
                string v;
                bool ok = database.hashes[name].get(key, v);
                result = ok ? v : "KEY_NOT_FOUND";
            } else {
                result = "HASH_NOT_EXISTS";
            }
        }
        else if (cmd == "TINSERT") {
            string name, value;
            ss >> name >> value;
            
            if (!database.trees.count(name)) {
                database.trees[name] = BinaryTree();
            }
            
            database.trees[name].insert(value);
            result = "OK"; 
            is_write_operation = true;
        }
        else if (cmd == "TFIND") {
            string name, value;
            ss >> name >> value;
            
            if (database.trees.count(name)) {
                string path;
                int position;
                bool ok = database.trees[name].find(value, path, position);
                
                if (ok) {
                    result = path + " " + to_string(position);
                } else {
                    result = "NOT_FOUND";
                }
            } else {
                result = "TREE_NOT_EXISTS";
            }
        }
        else {
            result = "ERROR: Unknown command " + cmd;
        }
    } // Освобождаем db_mutex здесь
    
    // автосохранение после операций записи
    if (is_write_operation && !last_save_path.empty()) {
        // Вызываем потокобезопасный метод save (он сам захватит нужные мьютексы)
        save(last_save_path);
    }
    
    return result;
}

