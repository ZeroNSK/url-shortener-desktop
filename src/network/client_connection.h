#pragma once
#include "../database/threadsafe_db.h"
#include "../utils/logger.h"
#include <string>
#include <sys/socket.h>
#include <unistd.h>

class ClientConnection {
private:
    int socket;
    ThreadSafeDB* database;
    bool owns_socket;
    
public:
    ClientConnection(int sock, ThreadSafeDB* db, bool owns_sock = true);
    ~ClientConnection();
    
    void processRequests();
    std::string readCommand();
    std::string executeCommand(const std::string& command);
    void sendResponse(const std::string& response);
};