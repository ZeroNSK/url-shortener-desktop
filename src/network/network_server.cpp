#include "network_server.h"
#include <iostream>
#include <cstring>
#include <errno.h>
#include <signal.h>
#include <sstream>


NetworkServer::NetworkServer(ThreadSafeDB* db) 
    : server_socket(-1), database(db), running(false), thread_pool(4) { // 4 рабочих потока
    signal(SIGPIPE, SIG_IGN); // игнорируем SIGPIPE чтобы не падать при записи в закрытый сокет
}
NetworkServer::~NetworkServer() {
    stop();
}
bool NetworkServer::start(int port) { 
    // создаем TCP сокет для входящих подключений
    server_socket = socket(AF_INET, SOCK_STREAM, 0); // сюда как раз IPv4 и TCP 

    if (server_socket < 0) {
        std::string error_msg = "Ошибка создания сокета: " + std::string(strerror(errno));
        Logger::getInstance().logError(error_msg, "NetworkServer::start");
        return false;
    }
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) { // быстрый рестарт на том же порту
        std::string error_msg = "Не удалось установить опции сокета: " + std::string(strerror(errno));
        Logger::getInstance().logError(error_msg, "NetworkServer::start");
        close(server_socket);
        server_socket = -1;
        return false;
    }
    
    // настройка структуры серва
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;           // IPv4
    server_addr.sin_addr.s_addr = INADDR_ANY;  // принимаем подключения с любых адресов
    server_addr.sin_port = htons(port);        // преобразование порта в сетевой формат
    
    //биндим что на таком-то порту сидит такой-то процесс
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) { 
        std::ostringstream oss; 
        oss << "Не удалось привязать сокет к порту " << port << ": " << strerror(errno);
        Logger::getInstance().logError(oss.str(), "NetworkServer::start");
        close(server_socket);
        server_socket = -1;
        return false;
    }
    
    //макс 10 ожидающих подключений
    if (listen(server_socket, 10) < 0) { 
        std::string error_msg = "Не удалось начать прослушивание сокета: " + std::string(strerror(errno));
        Logger::getInstance().logError(error_msg, "NetworkServer::start");
        close(server_socket);
        server_socket = -1;
        return false;
    }
    
    
    running = true;
    Logger::getInstance().logServerStart(port);
    return true;
}

void NetworkServer::stop() { // running меняем на false, закрываем сокет, завершаем пул потоков
    if (!running) {
        return;
    }
    
    running = false;
    if (server_socket >= 0) {
        close(server_socket);
        server_socket = -1;
    }
    thread_pool.shutdown();
    
    Logger::getInstance().logServerStop();
}

void NetworkServer::run() {
    if (!running) {
        Logger::getInstance().logError("Сервер не запущен", "NetworkServer::run");
        return;
    }
    
    acceptConnections();
}

void NetworkServer::acceptConnections() {
    while (running) {
        struct sockaddr_in client_addr; 
        socklen_t client_len = sizeof(client_addr); 
        
        int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len); // принимаем подключение под новым сокетом
        
        if (client_socket < 0) {
            if (running) {
                std::string error_msg = "Не удалось принять подключение: " + std::string(strerror(errno));
                Logger::getInstance().logError(error_msg, "NetworkServer::acceptConnections");
            }
            continue;
        }
        
        if (!running) {
            close(client_socket); 
            break;
        }
        
        std::cout << "DEBUG: Принят клиент, передача в ThreadPool" << std::endl; 
        
        // передаем обработку клиента в пул потоков
        thread_pool.enqueue([this, client_socket]() {
            handleClient(client_socket);
        });
        
        std::cout << "DEBUG: Клиент передан в ThreadPool" << std::endl;
    }
}

void NetworkServer::handleClient(int client_socket) { 
    std::cout << "DEBUG: handleClient вызван с сокетом " << client_socket << std::endl;
    ClientConnection client(client_socket, database, true); // когда уничтожим - закроем сокет
    
    std::cout << "DEBUG: ClientConnection создан, вызываем processRequests" << std::endl;

    client.processRequests();
    std::cout << "DEBUG: processRequests завершен" << std::endl;
}

