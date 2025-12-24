#include "logger.h"
#include <iostream>

Logger* Logger::instance = nullptr;
std::mutex Logger::instance_mutex;

Logger::Logger() : min_level(LogLevel::INFO) {
    // по умолчанию логируем в файл server.log
    setLogFile("server.log");
}
Logger& Logger::getInstance() {
    std::lock_guard<std::mutex> lock(instance_mutex); // защита от многопоточного доступа
    if (instance == nullptr) {
        instance = new Logger(); // создаем единственный экземпляр
    }
    return *instance;
}
void Logger::setLogFile(const std::string& filename) { // проверяем доступ потом закрываем открываем файл
    std::lock_guard<std::mutex> lock(log_mutex); 
    if (log_file.is_open()) {
        log_file.close();
    }
    log_file.open(filename, std::ios::app);
}
void Logger::setMinLevel(LogLevel level) { // устанавливаем минимальный уровень логирования
    min_level = level;
}
void Logger::log(LogLevel level, const std::string& message) { // основной метод логирования
    if (level < min_level) { 
        return;
    }
    std::lock_guard<std::mutex> lock(log_mutex); // защита от одновременного доступа из разных потоков
    
    std::string timestamp = getCurrentTimestamp(); // время
    std::string level_str = levelToString(level); // уровень
    
    std::string log_entry = "[" + timestamp + "] [" + level_str + "] " + message;
    
    // Логируем в файл
    if (log_file.is_open()) {
        log_file << log_entry << std::endl;
        log_file.flush();
    }
    
    // Также выводим в консоль для отладки
    std::cout << log_entry << std::endl;
}

void Logger::info(const std::string& message) {
    log(LogLevel::INFO, message);
}

void Logger::warning(const std::string& message) {
    log(LogLevel::WARNING, message);
}

void Logger::error(const std::string& message) {
    log(LogLevel::ERROR, message);
}

void Logger::debug(const std::string& message) {
    log(LogLevel::DEBUG, message);
}

void Logger::logServerStart(int port) {
    std::ostringstream oss;
    oss << "NetworkServer запущен на порту " << port;
    info(oss.str());
}

void Logger::logServerStop() {
    info("NetworkServer остановлен корректно");
}

void Logger::logClientConnect(const std::string& client_info) {
    std::ostringstream oss;
    oss << "Клиент подключен: " << client_info;
    info(oss.str());
}

void Logger::logClientDisconnect(const std::string& client_info) {
    std::ostringstream oss;
    oss << "Клиент отключен: " << client_info;
    info(oss.str());
}

void Logger::logDatabaseOperation(const std::string& command, const std::string& result) {
    std::ostringstream oss;
    oss << "Операция с базой данных - Команда: '" << command << "', Результат: '" << result << "'";
    info(oss.str());
}

void Logger::logError(const std::string& error_message, const std::string& context) {
    std::ostringstream oss;
    oss << "ERROR: " << error_message;
    if (!context.empty()) {
        oss << " (Контекст: " << context << ")";
    }
    error(oss.str());
}

std::string Logger::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    oss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    
    return oss.str();
}

std::string Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARNING: return "WARN";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::DEBUG: return "DEBUG";
        default: return "UNKNOWN";
    }
}