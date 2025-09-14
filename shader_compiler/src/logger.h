#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <memory>
#include <mutex>
#include <sstream>

enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    ERROR = 3
};

class Logger {
public:
    static Logger& Instance();
    
    void Initialize(const std::string& log_directory);
    void SetLogLevel(LogLevel level);
    
    void Log(LogLevel level, const std::string& message);
    void LogInfo(const std::string& message);
    void LogWarning(const std::string& message);
    void LogError(const std::string& message);
    void LogDebug(const std::string& message);
    
    // 专门用于记录编译错误的方法
    void LogCompilationError(const std::string& filename, const std::string& error);
    void LogCompilationSuccess(const std::string& filename, double time_ms);
    
    void Flush();

private:
    Logger() = default;
    ~Logger();
    
    std::string GetCurrentTimeString();
    std::string LogLevelToString(LogLevel level);
    void WriteToFile(const std::string& message);
    void WriteToConsole(const std::string& message, LogLevel level);
    
    std::unique_ptr<std::ofstream> m_log_file;
    std::unique_ptr<std::ofstream> m_error_file;
    std::mutex m_mutex;
    LogLevel m_current_level = LogLevel::INFO;
    std::string m_log_directory;
    bool m_initialized = false;
};

// 便捷宏
#define LOG_INFO(msg) Logger::Instance().LogInfo(msg)
#define LOG_WARNING(msg) Logger::Instance().LogWarning(msg)
#define LOG_ERROR(msg) Logger::Instance().LogError(msg)
#define LOG_DEBUG(msg) Logger::Instance().LogDebug(msg)

#endif // LOGGER_H
