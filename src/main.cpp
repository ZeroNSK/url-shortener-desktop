#include <iostream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <locale.h>
#include <signal.h>
#include <csignal>
#include <thread>
#include <atomic>
#include "database/database.h"
#include "database/threadsafe_db.h"
#include "network/network_server.h"
#include "utils/logger.h"

using namespace std;

// для обработки сигналов
NetworkServer* global_server = nullptr;
std::atomic<bool> should_exit(false);

// обработчик сигналов
void signalHandler(int signal) {
    cout << "\nПолучен сигнал " << signal << ", завершаем работу сервера..." << endl;
    should_exit = true;
    if (global_server) {
        global_server->stop();
    }
    exit(0); // Принудительный выход
}


int main(int argc, char** argv) {
    setlocale(LC_ALL, "RU");
    
    string filepath;      
    string query;       
    bool server_mode = false; 
    int port = 6379; 

    for (int i = 1; i < argc; i++) {
        string arg = argv[i];

        if (arg == "--file" && i + 1 < argc) { 
            filepath = argv[++i]; // следующий аргумент - путь к файлу БД
        }
        else if (arg == "--query" && i + 1 < argc) {
            query = argv[++i]; // следующий аргумент - SQL-подобная команда
        }
        else if (arg == "--server") {
            server_mode = true; // включаем режим сетевого сервера
        }
        else if (arg == "--port" && i + 1 < argc) {
            port = atoi(argv[++i]); // следующий аргумент - номер порта
        }
    }

    if (server_mode) {
        if (filepath.empty()) {
            filepath = "data/db.json";
        }

        Logger::getInstance().setLogFile("server.log");
        Logger::getInstance().setMinLevel(LogLevel::INFO);
        ThreadSafeDB database;
        
        if (!database.load(filepath)) {
            Logger::getInstance().logError("Не удалось загрузить базу данных из файла: " + filepath);
            cout << "Предупреждение: не удалось загрузить базу данных из " << filepath << ", создается новая" << endl;
        }

        NetworkServer server(&database);
        global_server = &server; // обработка сигналов

        signal(SIGINT, signalHandler);   // выход с проги
        signal(SIGTERM, signalHandler);
        
        cout << "Запуск сетевого сервера базы данных..." << endl;
        cout << "Файл базы данных: " << filepath << endl;
        cout << "Порт: " << port << endl;
        cout << "Для остановки нажмите Ctrl+C" << endl;

        if (!server.start(port)) {
            Logger::getInstance().logError("Не удалось запустить сервер на порту " + to_string(port));
            cout << "Ошибка: не удалось запустить сервер на порту " << port << endl;
            return 1;
        }

        // Запускаем сервер в отдельном потоке
        std::thread server_thread([&server]() {
            server.run();
        });

        // Интерактивный режим - можно вводить команды прямо в терминале
        cout << "\nИнтерактивный режим активен. Введите команды:" << endl;
        cout << "Примеры: SADD myset value1, HSET myhash key value, exit для выхода" << endl;
        cout << "> ";
        
        string command;
        while (!should_exit && getline(cin, command)) {
            if (should_exit || command == "exit" || command == "quit") {
                break;
            }
            
            if (!command.empty()) {
                try {
                    string result = database.executeCommand(command);
                    cout << result << endl;
                } catch (const exception& e) {
                    cout << "ERROR: " << e.what() << endl;
                }
            }
            cout << "> ";
        }
        
        cout << "\nЗавершение работы сервера..." << endl;
        server.stop();
        
        // Ждем завершения серверного потока
        if (server_thread.joinable()) {
            server_thread.join();
        }

        if (!database.save(filepath)) {
            Logger::getInstance().logError("Не удалось сохранить базу данных в файл: " + filepath);
            cout << "Предупреждение: не удалось сохранить базу данных в " << filepath << endl;
        }

        cout << "Сервер завершил работу" << endl;
        return 0;
    }


    if (filepath.empty() || query.empty()) {
        cout << "Использование:" << endl;
        cout << "  CLI режим: --file <путь к файлу> --query <команда>" << endl;
        cout << "  Сетевой режим: --server [--file <путь к файлу>] [--port <порт>]" << endl;
        return 1;
    }

    Database db;
    db.load(filepath);

    istringstream ss(query);
    string cmd;
    ss >> cmd;  

    string result;

    
    if (cmd == "SADD") {
        string name, value;
        ss >> name >> value;

        if (!db.sets.count(name)) {
            db.sets[name] = Set(); 
        }

        bool ok = db.sets[name].add(value);
        result = ok ? value : "DUPLICATE";
    } 
    else if (cmd == "SREM") {  
        string name, value;
        ss >> name >> value;

        if (db.sets.count(name)) {
            bool ok = db.sets[name].remove(value);
            result = ok ? value : "";
        }
    }
    else if (cmd == "SISMEMBER") { 
        string name, value;
        ss >> name >> value;

        if (db.sets.count(name)) {
            bool exists = db.sets[name].isMember(value);
            result = exists ? "TRUE" : "FALSE";
        } else {
            result = "FALSE"; 
        }
    }

    
    else if (cmd == "SPUSH") {
        string name, value;
        ss >> name >> value;

        if (!db.stacks.count(name)) {
            db.stacks[name] = Stack();
        }

        db.stacks[name].push(value); // добавляем элемент на вершину стека
        result = value; 
    }

    else if (cmd == "SPOP") {
        string name;
        ss >> name;

        if (db.stacks.count(name)) {
            string v;
            bool ok = db.stacks[name].pop(v); 
            result = ok ? v : "";
        }
    }
    
    else if (cmd == "QPUSH") {
        string name, value;
        ss >> name >> value;

        if (!db.queues.count(name)) {
            db.queues[name] = Queue();
        }

        db.queues[name].push(value); // Добавляем элемент в конец очереди
        result = value; 
    }

    else if (cmd == "QPOP") {
        string name;
        ss >> name;

        if (db.queues.count(name)) {
            string v;
            bool ok = db.queues[name].pop(v); // Извлекаем первый добавленный элемент
            result = ok ? v : "";
        }
    }
    
    else if (cmd == "HSET") {
        string name, key, value;
        ss >> name >> key >> value;

        if (!db.hashes.count(name)) {
            db.hashes[name] = Hash();
        }

        db.hashes[name].set(key, value); // Устанавливаем или обновляем значение по ключу
        result = value; 
    }

    else if (cmd == "HDEL") {
        string name, key;
        ss >> name >> key;

        if (db.hashes.count(name)) {
            bool ok = db.hashes[name].del(key); 
            result = ok ? key : ""; 
        }
    }

    else if (cmd == "HGET") {
        string name, key;
        ss >> name >> key;

        if (db.hashes.count(name)) {
            string v;
            bool ok = db.hashes[name].get(key, v); 
            result = ok ? v : ""; 
        }
    }

    
    else if (cmd == "TINSERT") {
        string name, value;
        ss >> name >> value;

        if (!db.trees.count(name)) {
            db.trees[name] = BinaryTree();
        }

        db.trees[name].insert(value);
        result = value;
    }

    else if (cmd == "TFIND") {
        string name, value;
        ss >> name >> value;

        if (db.trees.count(name)) {
            string path;
            int position;
            bool ok = db.trees[name].find(value, path, position); // Ищем элемент в дереве
            
            if (ok) {
                result = path + " " + to_string(position); // Возвращаем путь к элементу и позицию
            } else {
                result = "";
            }
        }
    }

    else {
        cout << "Ошибка: неизвестная команда " << cmd << endl;
        return 1;
    }
    db.save(filepath);
    cout << result << endl;

    return 0;
}