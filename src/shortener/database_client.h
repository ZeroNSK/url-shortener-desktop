#pragma once

#include <string>
#include <optional>

class DatabaseClient {
public:
    DatabaseClient(const std::string& host, int port);
    ~DatabaseClient();
    
    bool connect();
    void disconnect();
    bool isConnected() const;
    
    // создание записи short_code -> original_url
    bool setURL(const std::string& short_code, const std::string& original_url);
    
    // получение original_url по short_code
    std::optional<std::string> getURL(const std::string& short_code);
    
private:
    std::string host_;
    int port_;
    int socket_fd_;
    
    // Отправляет команду и получает ответ
    std::string sendCommand(const std::string& command);
};