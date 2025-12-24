#include "http_server.h"
#include "request_handler.h"
#include "../shortener/url_shortener_service.h"
#include "../network/thread_pool.h"
#include "../utils/logger.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <cstring>
#include <thread>
#include <vector>

using namespace std;

HTTPServer::HTTPServer(int port, URLShortenerService* service, size_t thread_pool_size)
    : port_(port), server_socket_(-1), service_(service), running_(false) {
    thread_pool_ = make_unique<ThreadPool>(thread_pool_size); // пул потоков где обрабатываются клиенты
}

HTTPServer::~HTTPServer() {
    stop();
    if (thread_pool_) {
        thread_pool_->shutdown();
    }
}

bool HTTPServer::start() {
    server_socket_ = socket(AF_INET, SOCK_STREAM, 0); // ipv4, tcp
    if (server_socket_ < 0) return false;
    
    int opt = 1;
    if (setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) { // быстрый рестарт
        close(server_socket_);
        return false;
    }
    
    struct sockaddr_in server_addr; // структура адреса сервера
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET; // ipv4
    server_addr.sin_addr.s_addr = INADDR_ANY; // все интерфейсы
    server_addr.sin_port = htons(port_); // сетевой порядок байт
    
    //привязываем сокет к порту
    if (::bind(server_socket_, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) { 
        close(server_socket_);
        return false;
    }
    
    // макс очередь
    if (listen(server_socket_, 10) < 0) {
        close(server_socket_);
        return false;
    }
    
    running_ = true;
    
    while (running_) {
        struct sockaddr_in client_addr; // структура адреса клиента
        socklen_t client_len = sizeof(client_addr); // размер структуры
        
        // принимаем подключение
        int client_socket = accept(server_socket_, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket < 0) {
            if (running_) continue;
        }
        // передаем обработку клиента в пул потоков
        thread_pool_->enqueue([this, client_socket]() {
            this->handleClient(client_socket);
        });
    }
    
    return true;
}

void HTTPServer::stop() {
    running_ = false;
    if (server_socket_ >= 0) { 
        close(server_socket_);
        server_socket_ = -1; // закрываем сокет сервераы
    }
}

void HTTPServer::handleClient(int client_socket) {
    const int BUFFER_SIZE = 4096; // буфер под 1 часть запроса клиента
    char buffer[BUFFER_SIZE];
    
    try {
        ssize_t bytes_read = recv(client_socket, buffer, BUFFER_SIZE - 1, 0); // читаем запрос
        if (bytes_read <= 0) {
            close(client_socket);
            return;
        }
        
        buffer[bytes_read] = '\0';
        string raw_request(buffer); 
        
        HTTPRequest request = parseRequest(raw_request); // превращаем в структуру запроса
        RequestHandler handler(service_); // обработчик запросов
        HTTPResponse response; 
        
        if (request.method == "POST" && request.path == "/") { // сокращение URL
            response = handler.handleShortenURL(request);
        } else if (request.method == "GET" && request.path.length() > 1) { // редирект 
            response = handler.handleRedirect(request);
        } else {
            response = handler.handleNotFound();
        }
        
        string http_response = buildResponse(response.status_code, response.body, response.headers);
        send(client_socket, http_response.c_str(), http_response.length(), 0); // отправляем ответ клиенту
        
    } catch (const exception& e) { // если че то не то 
        RequestHandler handler(service_);
        HTTPResponse error_response = handler.handleError(400, "Bad Request"); 
        string http_response = buildResponse(error_response.status_code, error_response.body, error_response.headers);
        send(client_socket, http_response.c_str(), http_response.length(), 0);
    }
    
    close(client_socket);
}

HTTPRequest HTTPServer::parseRequest(const string& raw_request) {
    HTTPRequest request;
    
    istringstream stream(raw_request); // читаем запрос 
    string line;
    
    if (!getline(stream, line)) { 
        throw runtime_error("Неверный HTTP запрос"); 
    }
    
    if (!line.empty() && line.back() == '\r') { 
        line.pop_back();
    }
    
    istringstream request_line(line); 
    string http_version;
    
    if (!(request_line >> request.method >> request.path >> http_version)) {
        throw runtime_error("Неверный HTTP запрос");
    }
    
    while (getline(stream, line) && !line.empty() && line != "\r") { 
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        
        size_t colon_pos = line.find(':');
        if (colon_pos != string::npos) {
            string header_name = line.substr(0, colon_pos);
            string header_value = line.substr(colon_pos + 1);
            
            size_t start = header_value.find_first_not_of(" \t");
            if (start != string::npos) {
                header_value = header_value.substr(start);
            }
            
            request.headers[header_name] = header_value; // сохраняем заголовок
        }
    }
    
    string body_line;
    while (getline(stream, body_line)) {
        request.body += body_line; // склеиваем тело запроса
        if (stream.peek() != EOF) {
            request.body += "\n";
        }
    }
    
    return request;
}

string HTTPServer::buildResponse(int status_code, 
                                const string& body,
                                const map<string, string>& headers) {
    ostringstream response;
    
    response << "HTTP/1.1 " << status_code << " ";
    
    switch (status_code) {
        case 200: response << "Ок"; break;
        case 301: response << "Перемещено навсегда"; break;
        case 400: response << "Неверный запрос"; break;
        case 404: response << "Не найдено"; break;
        case 503: response << "Сервис недоступен"; break;
        default: response << "Неизвестно"; break;
    }
    
    response << "\r\n";
    
    for (const auto& header : headers) { // печать всех заголовков
        response << header.first << ": " << header.second << "\r\n";
    }
    
    if (headers.find("Content-Length") == headers.end()) { // если нет заголовка длины контента добавляем его
        response << "Content-Length: " << body.length() << "\r\n";
    }
    
    if (headers.find("Connection") == headers.end()) { // закрываем соединение после ответа
        response << "Connection: close\r\n";
    }
    
    response << "\r\n";
    response << body;
    
    return response.str();
}