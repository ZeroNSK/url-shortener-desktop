#pragma once
#include "../database/threadsafe_db.h"
#include "client_connection.h"
#include "thread_pool.h"
#include "../utils/logger.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <atomic>

class NetworkServer {
private:
    int server_socket;
    ThreadSafeDB* database;
    std::atomic<bool> running;
    ThreadPool thread_pool;
    
public:
    NetworkServer(ThreadSafeDB* db);
    ~NetworkServer();
    
    bool start(int port = 6379);
    void stop();
    void run();
    
private:
    void acceptConnections();
    void handleClient(int client_socket);
};