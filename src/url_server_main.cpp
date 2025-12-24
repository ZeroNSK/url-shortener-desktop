#include <iostream>
#include <string>
#include <csignal>
#include <thread>
#include <atomic>
#include <chrono>
#include "http/http_server.h"
#include "shortener/url_shortener_service.h"
#include "shortener/database_client.h"
#include "utils/logger.h"

using namespace std;


HTTPServer* global_http_server = nullptr;
atomic<bool> should_exit(false); // потоки могут менять/читать флаг одновременно

// когда cmd+с в терминале
void signalHandler(int signal) {
    cout << "\nПолучен сигнал " << signal << ", завершаем работу сервера..." << endl;
    should_exit = true;
    if (global_http_server) {
        global_http_server->stop();
    }
}

int main() {
    int http_port = 8080; // порт хттп
    string db_host = "127.0.0.1"; // потому что локально
    int db_port = 6379; // по 2 практике по дефолту 6379
    size_t thread_pool_size = 4; // количество потоков в пуле для обработки клиентов
    
    // куда уходят логи и их уровень 
    Logger::getInstance().setLogFile("url_shortener.log");
    Logger::getInstance().setMinLevel(LogLevel::INFO);
    
    cout << "URL Shortener Server" << endl;
    cout << "HTTP сервер: порт " << http_port << endl;
    cout << "База данных: " << db_host << ":" << db_port << endl;
    cout << "Для остановки нажмите Ctrl+C" << endl;
    cout << endl;
    
    DatabaseClient db_client(db_host, db_port); // клиент бд
    
    cout << "Подключение к базе данных..." << endl;
    if (!db_client.connect()) {
        cerr << "Ошибка: не удалось подключиться к базе данных " << db_host << ":" << db_port << endl;
        return 1;
    }
    cout << "Подключение к базе данных успешно" << endl;
    
    // создаем hhtp сервер 
    URLShortenerService url_service(&db_client);
    HTTPServer http_server(http_port, &url_service, thread_pool_size);
    global_http_server = &http_server;
    
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    cout << "Запуск HTTP сервера..." << endl;
    if (!http_server.start()) {
        cerr << "Ошибка: не удалось запустить HTTP сервер на порту " << http_port << endl;
        return 1;
    }
    
    cout << "URL Shortener Server запущен!" << endl;
    cout << "API доступно по адресу: http://localhost:" << http_port << endl;
    cout << endl;
    
    while (!should_exit) {
        this_thread::sleep_for(chrono::milliseconds(100));
    }
    
    cout << "\nЗавершение работы сервиса..." << endl;
    http_server.stop();
    db_client.disconnect();
    cout << "URL Shortener Server завершил работу" << endl;
    
    return 0;
}