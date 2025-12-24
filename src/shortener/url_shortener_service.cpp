#include "url_shortener_service.h"
#include "database_client.h"
#include "../utils/logger.h"
#include <regex>
#include <random>
#include <algorithm>
#include <functional>
#include <iomanip>
#include <sstream>

using namespace std;

URLShortenerService::URLShortenerService(DatabaseClient* db_client)
    : db_client_(db_client) {
    if (!db_client_) {
        Logger::getInstance().logError("URLShortenerService создан с null указателем на DatabaseClient", "URLShortenerService::URLShortenerService");
    }
}

string URLShortenerService::shortenURL(const string& original_url) {
    try {
        Logger::getInstance().info("Начало сокращения URL: " + original_url);
        
        if (!db_client_) {
            Logger::getInstance().logError("DatabaseClient не инициализирован");
            return "";
        }
        
        if (!isValidURL(original_url)) {
            Logger::getInstance().logError("Неверный формат URL: " + original_url);
            return "";
        }
        
        auto existing_code = findExistingCode(original_url);
        if (existing_code.has_value()) {
            Logger::getInstance().info("Найден существующий код: " + existing_code.value());
            return existing_code.value();
        }
        
        string deterministic_code = generateCode(original_url, true);
        Logger::getInstance().info("Сгенерирован детерминированный код: " + deterministic_code);
        
        if (isCodeUnique(deterministic_code)) {
            if (db_client_->setURL(deterministic_code, original_url)) {
                Logger::getInstance().info("Детерминированный код сохранен в БД");
                return deterministic_code;
            }
        } else {
            auto existing_url = db_client_->getURL(deterministic_code);
            if (existing_url.has_value() && existing_url.value() == original_url) {
                Logger::getInstance().info("Детерминированный код уже существует для этого URL");
                return deterministic_code;
            }
        }
        
        string short_code;
        int max_attempts = 10;
        Logger::getInstance().info("Начинаем генерацию случайного кода (максимум " + to_string(max_attempts) + " попыток)");
        
        for (int attempt = 0; attempt < max_attempts; ++attempt) {
            short_code = generateCode("", false);
            Logger::getInstance().info("Попытка " + to_string(attempt + 1) + ": сгенерирован код " + short_code);
            
            if (isCodeUnique(short_code)) {
                if (db_client_->setURL(short_code, original_url)) {
                    Logger::getInstance().info("Случайный код успешно сохранен в БД");
                    return short_code;
                }
            } else {
                Logger::getInstance().info("Код не уникален, пробуем снова");
            }
        }
        
        Logger::getInstance().logError("Не удалось создать уникальный короткий код после " + 
                 to_string(max_attempts) + " попыток");
        return "";
        
    } catch (const exception& e) {
        Logger::getInstance().logError("Исключение при сокращении URL: " + string(e.what()));
        return "";
    }
}

optional<string> URLShortenerService::findExistingCode(const string& original_url) {
    try {
        if (!db_client_ || original_url.empty()) {
            return nullopt;
        }
        
        string deterministic_code = generateCode(original_url, true);
        auto existing_url = db_client_->getURL(deterministic_code);
        if (existing_url.has_value() && existing_url.value() == original_url) {
            return deterministic_code;
        }
        
        return nullopt;
        
    } catch (const exception& e) {
        Logger::getInstance().logError("Исключение при поиске существующего кода: " + string(e.what()), 
                 "URLShortenerService::findExistingCode");
        return nullopt;
    }
}

optional<string> URLShortenerService::getOriginalURL(const string& short_code) {
    try {
        if (!db_client_ || short_code.empty()) {
            return nullopt;
        }
        
        return db_client_->getURL(short_code);
        
    } catch (const exception& e) {
        Logger::getInstance().logError("Исключение при получении оригинального URL: " + string(e.what()), 
                 "URLShortenerService::getOriginalURL");
        return nullopt;
    }
}

bool URLShortenerService::isValidURL(const string& url) {
    try {
        if (url.empty() || url.length() > 2048) {
            return false;
        }
        
        regex url_regex(
            R"(^(https?|ftp)://[^\s/$.?#].[^\s]*$)",
            regex_constants::icase
        );
        
        return regex_match(url, url_regex);
        
    } catch (const exception& e) {
        Logger::getInstance().logError("Исключение при валидации URL: " + string(e.what()), "URLShortenerService::isValidURL");
        return false;
    }
}

string URLShortenerService::generateCode(const string& seed, bool deterministic) {
    try {
        string code;
        
        if (deterministic && !seed.empty()) {
            Logger::getInstance().info("Генерация детерминированного кода для: " + seed);
            
            // вычисляем сумму ASCII кодов
            unsigned long ascii_sum = 0;
            stringstream ascii_log;
            ascii_log << "ASCII коды символов: ";
            
            for (size_t i = 0; i < seed.length(); ++i) {
                unsigned char ascii_val = static_cast<unsigned char>(seed[i]);
                ascii_sum += ascii_val;
                ascii_log << seed[i] << "=" << static_cast<int>(ascii_val);
                if (i < seed.length() - 1) ascii_log << ", ";
            }
            
            Logger::getInstance().info(ascii_log.str());
            Logger::getInstance().info("Сумма ASCII кодов: " + to_string(ascii_sum));
            
            // модифицируем сумму длиной строки
            ascii_sum = ascii_sum * seed.length() + seed.length();
            Logger::getInstance().info("Модифицированная сумма (сумма * длина + длина): " + to_string(ascii_sum));
            
            // генерируем код в системе счисления base62
            unsigned long temp_sum = ascii_sum;
            stringstream generation_log;
            generation_log << "Генерация символов в base62: ";
            
            for (int i = 0; i < 6; ++i) {
                int char_index = temp_sum % 62;
                char generated_char = indexToChar(char_index);
                code = generated_char + code;
                
                generation_log << "позиция " << i << ": " << temp_sum << " % 62 = " << char_index 
                              << " -> '" << generated_char << "'";
                
                temp_sum /= 62;
                
                if (temp_sum == 0 && i < 5) {
                    temp_sum = static_cast<unsigned char>(seed[i % seed.length()]) + ascii_sum;
                    generation_log << " (обновили temp_sum до " << temp_sum << ")";
                }
                
                if (i < 5) generation_log << ", ";
            }
            
            Logger::getInstance().info(generation_log.str());
            Logger::getInstance().info("Итоговый детерминированный код: " + code);
            
        } else {
            Logger::getInstance().info("Генерация случайного кода");
            
            static random_device rd;
            static mt19937 gen(rd());
            uniform_int_distribution<> char_dis(0, 61);
            uniform_int_distribution<> length_dis(6, 8);
            int code_length = length_dis(gen);
            
            Logger::getInstance().info("Длина случайного кода: " + to_string(code_length));
            
            code.reserve(code_length);
            stringstream random_log;
            random_log << "Генерация случайных символов: ";
            
            for (int i = 0; i < code_length; ++i) {
                int char_index = char_dis(gen);
                char generated_char = indexToChar(char_index);
                code += generated_char;
                
                random_log << "позиция " << i << ": индекс " << char_index << " -> '" << generated_char << "'";
                if (i < code_length - 1) random_log << ", ";
            }
            
            Logger::getInstance().info(random_log.str());
            Logger::getInstance().info("Итоговый случайный код: " + code);
        }
        
        return code;
        
    } catch (const exception& e) {
        Logger::getInstance().logError("Исключение при генерации кода: " + string(e.what()), 
                 "URLShortenerService::generateCode");
        return deterministic ? "ascii1" : "error1";
    }
}

char URLShortenerService::indexToChar(int index) {
    if (index < 26) {
        return 'a' + index;
    } else if (index < 52) {
        return 'A' + (index - 26);
    } else {
        return '0' + (index - 52);
    }
}

bool URLShortenerService::isCodeUnique(const string& code) {
    try {
        if (!db_client_ || code.empty()) {
            return false;
        }
        
        auto existing_url = db_client_->getURL(code);
        return !existing_url.has_value();
        
    } catch (const exception& e) {
        Logger::getInstance().logError("Исключение при проверке уникальности кода: " + string(e.what()), 
                 "URLShortenerService::isCodeUnique");
        return false;
    }
} 