#pragma once
#include "database.h"
#include "../utils/logger.h"
#include <mutex>
#include <string>
#include <sstream>

using std::mutex;
using std::lock_guard;
using std::string;
using std::istringstream;

class ThreadSafeDB { // обработка потокобезопасных операций с БД
private:
    Database database;
    mutable mutex db_mutex;      // для синхронизации доступа к структурам данных
    mutable mutex file_mutex;    // для синхронизации операций с файлом
    string last_save_path;       // путь к файлу для автосохранения
    
public:
    ThreadSafeDB();
    
    // потокобезопасные операции с файлом
    bool load(const string& path);
    bool save(const string& path);
    
    // основной метод для выполнения команд
    string executeCommand(const string& command);
    

};