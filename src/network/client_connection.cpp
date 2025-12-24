#include "client_connection.h"
#include <iostream>
#include <cstring>
#include <errno.h>
#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>
#include <iomanip>

using std::cout, std::endl;

// сохранение переданных значений
ClientConnection::ClientConnection(int sock, ThreadSafeDB* db, bool owns_sock) 
    : socket(sock), database(db), owns_socket(owns_sock) {
}

ClientConnection::~ClientConnection() {
    if (owns_socket && socket >= 0) { // закрываем сокет если владеем им
        close(socket);
    }
}

void ClientConnection::processRequests() {
    // получаем информацию о клиенте для логирования
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    std::string client_info = "unknown";
    
    if (getpeername(socket, (struct sockaddr*)&client_addr, &addr_len) == 0) { // если есть адрес юзера
        char ip_str[INET_ADDRSTRLEN]; 
        inet_ntop(AF_INET, &client_addr.sin_addr, ip_str, INET_ADDRSTRLEN); // превращаем с бинарника в строку
        std::ostringstream oss; // собираем строку вида айпи:порт
        oss << ip_str << ":" << ntohs(client_addr.sin_port); // делаем порт читаемым
        client_info = oss.str();
    }
    
    // Получаем ID текущего потока
    std::ostringstream thread_info;
    thread_info << std::this_thread::get_id();
    std::string thread_str = thread_info.str();
    
    std::cout << "DEBUG: [ПОТОК " << thread_str << "] Начинаем обработку запросов от " << client_info << endl;
    
    // Логируем с информацией о потоке
    std::string client_with_thread_info = client_info + " [ПОТОК: " + thread_str + "]";
    Logger::getInstance().logClientConnect(client_with_thread_info);
    
    while (true) { // вечный цикл - запрос, лог, ответ
        cout << "DEBUG: Ожидаем команду..." << endl;
        std::string command = readCommand();
        cout << "DEBUG: Получена команда: '" << command << "'" << endl;
        if (command.empty()) {
            cout << "DEBUG: Пустая команда, прерываем цикл" << endl;
            break;
        }
        cout << "DEBUG: Выполняем команду..." << endl;
        std::string result = executeCommand(command);
        
        cout << "DEBUG: [ПОТОК " << thread_str << "] Результат команды: '" << result << "'" << endl;
        
        // Добавляем информацию о потоке в лог операции
        std::string command_with_thread_info = "[ПОТОК " + thread_str + "] " + command;
        Logger::getInstance().logDatabaseOperation(command_with_thread_info, result);
        cout << "DEBUG: Отправляем ответ..." << endl;
        sendResponse(result);
        cout << "DEBUG: Ответ отправлен" << endl;
    }
    
    cout << "DEBUG: [ПОТОК " << thread_str << "] Завершаем processRequests" << endl;
    
    std::string client_with_thread_disconnect = client_info + " [ПОТОК: " + thread_str + "]";
    Logger::getInstance().logClientDisconnect(client_with_thread_disconnect);
}

std::string ClientConnection::readCommand() {
    char buffer[1024]; 
    cout << "DEBUG: Ожидаем команду..." << endl;
    
    ssize_t bytes_read = recv(socket, buffer, sizeof(buffer) - 1, 0); // читаем из сокета в буфер 
    
    cout << "DEBUG: recv() возвращено " << bytes_read << endl;
    
    if (bytes_read <= 0) {
        cout << "DEBUG: Клиент отключился или ошибка, bytes_read=" << bytes_read << endl;
        return "";
    }
    
    buffer[bytes_read] = '\0'; // завершаем нулем и делаем строку
    cout << "DEBUG: Получено: '" << std::string(buffer, bytes_read) << "'" << endl;
    
    // удаляем символы новой строки в конце
    std::string command(buffer);
    while (!command.empty() && (command.back() == '\n' || command.back() == '\r')) {
        command.pop_back();
    }
    cout << "DEBUG: Разобранная команда: '" << command << "'" << endl;
    return command;
}


std::string ClientConnection::executeCommand(const std::string& command) { // связь с бд
    cout << "DEBUG: executeCommand вызван с: '" << command << "'" << endl;
    if (command.empty()) {
        cout << "DEBUG: Команда пуста" << endl;
        return "ERROR: Пустая команда";
    }
    
    cout << "DEBUG: Готовимся вызвать database->executeCommand" << endl;
    
    // если какая-то фигня то сервер не падает а возвращает ошибку
    std::string result;
    try {
        result = database->executeCommand(command);
        cout << "DEBUG: database->executeCommand returned: '" << result << "'" << endl;
    } catch (const std::exception& e) {
        cout << "DEBUG: Исключение: " << e.what() << endl;
        return "ERROR: Экземпляр исключения: " + std::string(e.what());
    }
    // если результат пустой - возвращаем ошибку
    if (result.empty()) {
        cout << "DEBUG: Результат пуст" << endl;
        return "ERROR: Нет результата";
    }
    
    cout << "DEBUG: Возвращаем результат: '" << result << "'" << endl;
    return result;
}

void ClientConnection::sendResponse(const std::string& response) {
    std::string full_response = response + "\n"; // для удобства чтения 
    ssize_t bytes_sent = send(socket, full_response.c_str(), full_response.length(), 0); // отправляем в сокет
    
    if (bytes_sent < 0) {
        std::string error_msg = "Не удалось отправить ответ: " + std::string(strerror(errno));
        Logger::getInstance().logError(error_msg, "ClientConnection::sendResponse");
    }
}