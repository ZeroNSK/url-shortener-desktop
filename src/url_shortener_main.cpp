#include <iostream>
#include <string>
#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>


using namespace std;

void printUsage(const char* program_name) {
    cout << "Использование: " << program_name << " <URL>" << endl;
    cout << endl;
    cout << "Примеры:" << endl;
    cout << "  " << program_name << " https://github.com/" << endl;
    cout << "  " << program_name << " \"https://example.com?param=value\"" << endl;
}

string sendHttpRequest(const string& host, int port, const string& url) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return "";
    
    // настраиваем подключение
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(host.c_str());
    
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        close(sock);
        return "";
    }
    
    stringstream request;
    string json_body = "{\"url\": \"" + url + "\"}"; // тело запроса в формате JSON
    
    // строим HTTP запрос на сервер 
    request << "POST / HTTP/1.1\r\n";
    request << "Host: " << host << ":" << port << "\r\n";
    request << "Content-Type: application/json\r\n";
    request << "Content-Length: " << json_body.length() << "\r\n";
    request << "Connection: close\r\n";
    request << "\r\n";
    request << json_body;
    
    // отправляем запрос
    string request_str = request.str();
    send(sock, request_str.c_str(), request_str.length(), 0);
    
    char buffer[4096];
    string response;
    ssize_t bytes_read;
    
    // читаем ответ/его часть
    while ((bytes_read = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_read] = '\0';
        response += buffer;
    }
    
    close(sock);
    return response;
}

// достаем короткий код из ответа сервера
string extractShortCode(const string& response) {
    size_t start = response.find("\"short_code\":\"");
    if (start == string::npos) return "";
    start += 14;
    
    size_t end = response.find("\"", start);
    if (end == string::npos) return "";
    
    return response.substr(start, end - start);
}

int main(int argc, char** argv) {
    if (argc != 2) {
        printUsage(argv[0]);
        return 1;
    }
    
    string url = argv[1];
    
    if (url.find("http://") != 0 && url.find("https://") != 0) {
        cerr << "Ошибка: URL должен начинаться с http:// или https://" << endl;
        return 1;
    }
    
    cout << "Сокращаем URL: " << url << endl;
    
    string response = sendHttpRequest("127.0.0.1", 8080, url);
    
    if (response.empty()) {
        cerr << "Ошибка: не удалось подключиться к серверу на localhost:8080" << endl;
        cerr << "Убедитесь, что сервер запущен:" << endl;
        cerr << "  1. ./program --server --port 6379" << endl;
        cerr << "  2. ./url_server" << endl;
        return 1;
    }
    
    if (response.find("HTTP/1.1 200") == string::npos) {
        cerr << "Ошибка сервера:" << endl;
        size_t body_start = response.find("\r\n\r\n");
        if (body_start != string::npos) {
            cerr << response.substr(body_start + 4) << endl;
        }
        return 1;
    }
    
    string short_code = extractShortCode(response);
    if (short_code.empty()) {
        cerr << "Ошибка: не удалось получить короткий код" << endl;
        return 1;
    }
    
    cout << "Успешно сокращено!" << endl;
    
    string clickable_url = "http://localhost:8080/" + short_code;
    cout << "Короткая ссылка: \033]8;;" << clickable_url << "\033\\" << clickable_url << "\033]8;;\033\\" << endl;
    
    return 0;
}