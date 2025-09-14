#include "logger.h"
#include "file_utils.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <sstream>

Logger& Logger::Instance() {
    static Logger instance;
    return instance;
}

Logger::~Logger() {
    if (m_log_file && m_log_file->is_open()) {
        m_log_file->close();
    }
    if (m_error_file && m_error_file->is_open()) {
        m_error_file->close();
    }
}

void Logger::Initialize(const std::string& log_directory) {
    std::string log_filename, error_filename;
    
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        m_log_directory = log_directory;
        
        // 创建日志目录
        if (!FileUtils::DirectoryExists(log_directory)) {
            FileUtils::CreateDirectory(log_directory);
        }
        
        // 获取当前时间戳用于文件名
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        
        std::ostringstream timestamp;
        timestamp << std::put_time(std::localtime(&time_t), "%Y%m%d_%H%M%S");
        
        // 打开主日志文件
        log_filename = FileUtils::JoinPath(log_directory, "shader_compiler_" + timestamp.str() + ".log");
        m_log_file = std::make_unique<std::ofstream>(log_filename, std::ios::out | std::ios::app);
        
        if (!m_log_file->is_open()) {
            throw std::runtime_error("Unable to create log file: " + log_filename);
        }
        
        // 打开错误日志文件
        error_filename = FileUtils::JoinPath(log_directory, "compilation_errors.log");
        m_error_file = std::make_unique<std::ofstream>(error_filename, std::ios::out | std::ios::app);
        
        if (!m_error_file->is_open()) {
            throw std::runtime_error("Unable to create error log file: " + error_filename);
        }
        
        m_initialized = true;
    } // 锁在这里释放
    
    // 现在在锁外面写入初始化信息
    Log(LogLevel::INFO, "Log system initialized");
    Log(LogLevel::INFO, "Main log file: " + log_filename);
    Log(LogLevel::INFO, "Error log file: " + error_filename);
}

void Logger::SetLogLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_current_level = level;
}

void Logger::Log(LogLevel level, const std::string& message) {
    if (!m_initialized || level < m_current_level) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::string timestamp = GetCurrentTimeString();
    std::string level_str = LogLevelToString(level);
    
    std::ostringstream oss;
    oss << "[" << timestamp << "] [" << level_str << "] " << message;
    
    std::string formatted_message = oss.str();
    
    // 写入文件
    WriteToFile(formatted_message);
    
    // 写入控制台
    WriteToConsole(formatted_message, level);
}

void Logger::LogInfo(const std::string& message) {
    Log(LogLevel::INFO, message);
}

void Logger::LogWarning(const std::string& message) {
    Log(LogLevel::WARNING, message);
}

void Logger::LogError(const std::string& message) {
    Log(LogLevel::ERROR, message);
}

void Logger::LogDebug(const std::string& message) {
    Log(LogLevel::DEBUG, message);
}

void Logger::LogCompilationError(const std::string& filename, const std::string& error) {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (!m_error_file || !m_error_file->is_open()) {
            return;
        }
        
        std::string timestamp = GetCurrentTimeString();
        
        *m_error_file << "=====================================================" << std::endl;
        *m_error_file << "Compilation Error - " << timestamp << std::endl;
        *m_error_file << "File: " << filename << std::endl;
        *m_error_file << "=====================================================" << std::endl;
        *m_error_file << error << std::endl;
        *m_error_file << std::endl;
        
        m_error_file->flush();
    } // 锁在这里释放
    
    // 现在在锁外面记录到主日志
    Log(LogLevel::ERROR, "Compilation error details written to error log file: " + filename);
}

void Logger::LogCompilationSuccess(const std::string& filename, double time_ms) {
    std::ostringstream oss;
    oss << "Compilation successful: " << filename << " (time: " << std::fixed << std::setprecision(2) << time_ms << "ms)";
    Log(LogLevel::INFO, oss.str());
}

void Logger::Flush() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_log_file && m_log_file->is_open()) {
        m_log_file->flush();
    }
    
    if (m_error_file && m_error_file->is_open()) {
        m_error_file->flush();
    }
}

std::string Logger::GetCurrentTimeString() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    oss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    
    return oss.str();
}

std::string Logger::LogLevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG:   return "DEBUG";
        case LogLevel::INFO:    return "INFO ";
        case LogLevel::WARNING: return "WARN ";
        case LogLevel::ERROR:   return "ERROR";
        default:                return "UNKNW";
    }
}

void Logger::WriteToFile(const std::string& message) {
    if (m_log_file && m_log_file->is_open()) {
        *m_log_file << message << std::endl;
    }
}

void Logger::WriteToConsole(const std::string& message, LogLevel level) {
    // 根据日志级别选择输出流和颜色
    switch (level) {
        case LogLevel::ERROR:
            // 错误信息不在这里输出到控制台，因为主程序会处理
            break;
        case LogLevel::WARNING:
            // 警告信息也由主程序处理
            break;
        case LogLevel::DEBUG:
            if (m_current_level <= LogLevel::DEBUG) {
                std::cout << message << std::endl;
            }
            break;
        default:
            // INFO级别的日志由主程序控制是否输出
            break;
    }
}
