#pragma once

#include <string>
#include <optional>

class DatabaseClient;

class URLShortenerService {
public:
    URLShortenerService(DatabaseClient* db_client);
    
    std::string shortenURL(const std::string& original_url);
    std::optional<std::string> findExistingCode(const std::string& original_url);
    std::optional<std::string> getOriginalURL(const std::string& short_code);
    bool isValidURL(const std::string& url);
    
private:
    DatabaseClient* db_client_;
    
    std::string generateCode(const std::string& seed, bool deterministic);
    char indexToChar(int index);
    bool isCodeUnique(const std::string& code);
};