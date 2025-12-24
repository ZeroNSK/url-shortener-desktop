#include "database_client.h"
#include "../utils/logger.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include <iostream>
#include <errno.h>

using namespace std;

DatabaseClient::DatabaseClient(const string& host, int port)
    : host_(host), port_(port), socket_fd_(-1) {
}

DatabaseClient::~DatabaseClient() {
    disconnect();
}

bool DatabaseClient::connect() { 
    try {
        if (socket_fd_ >= 0) {
            return true;
        }
        
        socket_fd_ = socket(AF_INET, SOCK_STREAM, 0); // создаем tcp сокет
        if (socket_fd_ < 0) {
            Logger::getInstance().logError("Ошибка создания сокета: " + string(strerror(errno)), "DatabaseClient::connect");
            return false;
        }
        
        // создаем обнуляем и заполняем структуру адреса сервера
        struct sockaddr_in server_addr; 
        memset(&server_addr, 0, sizeof(server_addr)); 
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port_);
        
        if (inet_pton(AF_INET, host_.c_str(), &server_addr.sin_addr) <= 0) { // ip в бинарный вид
            Logger::getInstance().logError("Неверный адрес: " + host_, "DatabaseClient::connect");
            close(socket_fd_);
            socket_fd_ = -1;
            return false;
        }
        
        if (::connect(socket_fd_, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) { // подключаемся к серверу
            Logger::getInstance().logError("Не удалось подключиться к " + host_ + ":" + to_string(port_) + 
                     " - " + string(strerror(errno)), "DatabaseClient::connect");
            close(socket_fd_);
            socket_fd_ = -1;
            return false;
        }
        
        return true;
        
    } catch (const exception& e) {
        Logger::getInstance().logError("Исключение при подключении к БД: " + string(e.what()), "DatabaseClient::connect");
        if (socket_fd_ >= 0) {
            close(socket_fd_);
            socket_fd_ = -1;
        }
        return false;
    }
}

void DatabaseClient::disconnect() { // закрываем сокет
    if (socket_fd_ >= 0) {
        close(socket_fd_);
        socket_fd_ = -1;
    }
}

bool DatabaseClient::isConnected() const {
    return socket_fd_ >= 0;
}

// в хештаблице urls сохраняем шорт код и оригинальный урл
bool DatabaseClient::setURL(const string& short_code, const string& original_url) {
    try {
        if (!isConnected() || short_code.empty() || original_url.empty()) {
            return false;
        }
        
        ostringstream cmd;
        cmd << "HSET urls " << short_code << " " << original_url;
        
        string response = sendCommand(cmd.str());
        return (response == "OK");
        
    } catch (const exception& e) {
        Logger::getInstance().logError("Исключение при сохранении URL: " + string(e.what()), "DatabaseClient::setURL");
        return false;
    }
}


// получаем оригинальный урл по шорт коду из хештаблицы урлс
optional<string> DatabaseClient::getURL(const string& short_code) {
    try {
        if (!isConnected() || short_code.empty()) {
            return nullopt;
        }
        
        ostringstream cmd;
        cmd << "HGET urls " << short_code;
        
        string response = sendCommand(cmd.str());
        
        if (response == "KEY_NOT_FOUND" || response == "HASH_NOT_EXISTS" || response.empty()) {
            return nullopt;
        }
        
        return response;
        
    } catch (const exception& e) {
        Logger::getInstance().logError("Исключение при получении URL: " + string(e.what()), "DatabaseClient::getURL");
        return nullopt;
    }
}

// отправляем команду на сервер и получаем ответ
string DatabaseClient::sendCommand(const string& command) {
    try {
        if (!isConnected() || command.empty()) {
            return "";
        }
        
        string cmd_with_newline = command + "\n"; // команды завершаются новой строкой
        ssize_t bytes_sent = send(socket_fd_, cmd_with_newline.c_str(), cmd_with_newline.length(), 0); // отправляем команду
        
        if (bytes_sent < 0) {
            Logger::getInstance().logError("Ошибка отправки команды: " + string(strerror(errno)), "DatabaseClient::sendCommand");
            disconnect();
            return "";
        }
        
        char buffer[4096]; 
        ssize_t bytes_received = recv(socket_fd_, buffer, sizeof(buffer) - 1, 0); // получаем ответ
        
        if (bytes_received <= 0) {
            if (bytes_received == 0) {
                Logger::getInstance().logError("Соединение закрыто сервером БД", "DatabaseClient::sendCommand");
            } else {
                Logger::getInstance().logError("Ошибка получения ответа: " + string(strerror(errno)), "DatabaseClient::sendCommand");
            }
            disconnect();
            return "";
        }
        
        buffer[bytes_received] = '\0';
        
        string response(buffer);
        while (!response.empty() && (response.back() == '\n' || response.back() == '\r')) { // убираем переводы строк в конце
            response.pop_back();
        }
        
        return response;
        
    } catch (const exception& e) {
        Logger::getInstance().logError("Исключение при отправке команды БД: " + string(e.what()), "DatabaseClient::sendCommand");
        disconnect();
        return "";
    }
}