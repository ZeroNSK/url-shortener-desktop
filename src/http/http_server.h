#pragma once

#include <string>
#include <map>
#include <atomic>
#include <optional>
#include <memory>

class URLShortenerService;
class ThreadPool;

struct HTTPRequest {
    std::string method;   
    std::string path;      
    std::map<std::string, std::string> headers;
    std::string body;
};

struct HTTPResponse {
    int status_code;  
    std::map<std::string, std::string> headers;
    std::string body;
};

class HTTPServer {
public:
    HTTPServer(int port, URLShortenerService* service, size_t thread_pool_size = 4);
    ~HTTPServer();
    
    bool start();
    void stop();
    void handleClient(int client_socket);
    
private:
    int port_;
    int server_socket_;
    URLShortenerService* service_;
    std::atomic<bool> running_;
    std::unique_ptr<ThreadPool> thread_pool_;
    
    HTTPRequest parseRequest(const std::string& raw_request);
    std::string buildResponse(int status_code, 
                              const std::string& body,
                              const std::map<std::string, std::string>& headers);
};