#pragma once
#include <string>
#include <fstream>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <sstream>

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class Logger {
private:
    static Logger* instance;
    static std::mutex instance_mutex;
    
    std::ofstream log_file;
    std::mutex log_mutex;
    LogLevel min_level;
    
    Logger();
    
public:
    static Logger& getInstance();
    
    void setLogFile(const std::string& filename);
    void setMinLevel(LogLevel level);
    
    void log(LogLevel level, const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    void debug(const std::string& message);
    
    // специальные методы для логирования событий сети и базы данных
    void logServerStart(int port);
    void logServerStop();
    void logClientConnect(const std::string& client_info);
    void logClientDisconnect(const std::string& client_info);
    void logDatabaseOperation(const std::string& command, const std::string& result);
    void logError(const std::string& error_message, const std::string& context = "");
    
private:
    std::string getCurrentTimestamp();
    std::string levelToString(LogLevel level);
};

// макросы для удобного логирования
#define LOG_INFO(msg) Logger::getInstance().info(msg)
#define LOG_WARNING(msg) Logger::getInstance().warning(msg)
#define LOG_ERROR(msg) Logger::getInstance().error(msg)
#define LOG_DEBUG(msg) Logger::getInstance().debug(msg)