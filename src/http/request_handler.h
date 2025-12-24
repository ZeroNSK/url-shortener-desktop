#pragma once

#include "http_server.h"
#include <string>

// Forward declaration
class URLShortenerService;

class RequestHandler {
public:
    RequestHandler(URLShortenerService* service);
    
    HTTPResponse handleShortenURL(const HTTPRequest& request);
    HTTPResponse handleRedirect(const HTTPRequest& request);
    HTTPResponse handleNotFound();
    HTTPResponse handleError(int status_code, const std::string& message);
    
private:
    URLShortenerService* service_;
    
    std::string buildJSONResponse(const std::string& short_code);
    std::string buildErrorJSON(const std::string& error);
};