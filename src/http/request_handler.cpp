#include "request_handler.h"
#include "../shortener/url_shortener_service.h"
#include "../external/json.hpp"
#include <iostream>

using json = nlohmann::json;
using namespace std;

RequestHandler::RequestHandler(URLShortenerService* service)
    : service_(service) {
}

HTTPResponse RequestHandler::handleShortenURL(const HTTPRequest& request) {
    // входит url:ffff, выходит short_code:bbbb
    try {
        if (request.method != "POST") { // только пост запросы
            return handleError(400, "Метод не разрешен");
        }
        
        if (request.body.empty()) { // тело запроса не пустое
            return handleError(400, "Тело запроса пусто");
        }
        
        if (!service_) { // если сервис не nullptr
            return handleError(503, "Сервис временно недоступен");
        }
        
        json request_json = json::parse(request.body);
        
        if (!request_json.contains("url") || !request_json["url"].is_string()) { // проверяем что юрл и он строка
            return handleError(400, "Отсутствует или неверное поле 'url'");
        }
        
        string original_url = request_json["url"];
        
        if (original_url.empty()) { // что не пустой 
            return handleError(400, "URL не может быть пустым");
        }
        
        if (!service_->isValidURL(original_url)) { // проверяем формат урл
            return handleError(400, "Неверный формат URL");
        }
        
        string short_code = service_->shortenURL(original_url); // сокращаем 
        
        if (short_code.empty()) { // че то не так и строка пуста
            return handleError(503, "Сервис временно недоступен");
        }
        
        HTTPResponse response;
        response.status_code = 200; 
        response.headers["Content-Type"] = "application/json";
        response.body = buildJSONResponse(short_code); // все норм
        
        return response;
        
    } catch (const json::parse_error& e) {
        return handleError(400, "Неверный формат JSON"); // если че то сломано в джейсоне
    } catch (const exception& e) {
        return handleError(500, "Внутренняя ошибка сервера"); // все остальное 
    }
}

HTTPResponse RequestHandler::handleRedirect(const HTTPRequest& request) {
    try {
        if (request.method != "GET") { 
            return handleError(400, "Метод не разрешен");
        }
        
        string short_code;
        if (request.path.length() > 1 && request.path[0] == '/') { // путь должен начинаться с /
            short_code = request.path.substr(1);
        }
        
        if (short_code.empty()) {
            return handleNotFound(); // если короткий код пустой
        }
        
        if (!service_) {
            return handleError(503, "Сервис временно недоступен");
        }
        
        auto original_url = service_->getOriginalURL(short_code); // получаем оригинальный урл по короткому коду
        
        if (!original_url.has_value()) { // если не нашли
            return handleNotFound(); // 404
        }
        
        HTTPResponse response;
        response.status_code = 301; // переадресация 
        response.headers["Location"] = original_url.value(); // куда редиректить
        response.headers["Content-Length"] = "0"; // нет тела
        response.body = "";
        
        return response;
        
    } catch (const exception& e) {
        return handleError(500, "Внутренняя ошибка сервера");
    }
}

HTTPResponse RequestHandler::handleNotFound() { // 404
    HTTPResponse response;
    response.status_code = 404;
    response.headers["Content-Type"] = "application/json";
    response.body = buildErrorJSON("Short code не найден");
    
    return response;
}

HTTPResponse RequestHandler::handleError(int status_code, const string& message) { // общая ошибка
    HTTPResponse response;
    response.status_code = status_code;
    response.headers["Content-Type"] = "application/json";
    response.body = buildErrorJSON(message);
    
    return response;
}

string RequestHandler::buildJSONResponse(const string& short_code) { // строим джейсон ответ
    try {
        json response_json;
        response_json["short_code"] = short_code;
        return response_json.dump();
        
    } catch (const exception& e) {
        return R"({"error": "Не удалось сформировать ответ"})";
    }
}

string RequestHandler::buildErrorJSON(const string& error) { // строим джейсон ошибку
    try {
        json error_json;
        error_json["error"] = error;
        return error_json.dump();
        
    } catch (const exception& e) {
        return R"({"error": "Внутренняя ошибка сервера"})";
    }
}