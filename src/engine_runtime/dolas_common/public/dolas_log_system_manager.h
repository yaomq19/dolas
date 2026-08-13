#ifndef DOLAS_LOG_SYSTEM_MANAGER_H
#define DOLAS_LOG_SYSTEM_MANAGER_H

#include <string>
#include <memory>
#include <format>

namespace spdlog
{
    class logger;
}

namespace Dolas
{
    class LogSystemManager
    {
    public:
        LogSystemManager();
        ~LogSystemManager();

        bool Initialize();
        bool Clear();

        // 获取日志器实例（单例模式）
        static LogSystemManager& GetInstance();

        // 内部使用：获取 logger 指针（仅供宏使用，不要直接调用）
        void* GetLoggerPtr() const;
        
    private:
        std::shared_ptr<spdlog::logger> m_logger;
        std::string m_log_file_path;
    };
}

// 内部辅助函数声明（实现在 cpp 中，完全隐藏 spdlog）
namespace Dolas { namespace detail {
    void DoLogTrace(void* logger, const std::string& msg);
    void DoLogDebug(void* logger, const std::string& msg);
    void DoLogInfo(void* logger, const std::string& msg);
    void DoLogWarn(void* logger, const std::string& msg);
    void DoLogError(void* logger, const std::string& msg);
    void DoLogCritical(void* logger, const std::string& msg);

    // 格式化辅助：单参数直接转 string（兼容运行时字符串）
    inline std::string FormatMsg(const std::string& msg) { return msg; }
    inline std::string FormatMsg(const char* msg) { return std::string(msg); }

    // 格式化辅助：多参数走 std::format
    template<typename... Args>
    inline std::string FormatMsg(std::format_string<Args...> fmt, Args&&... args)
    {
        return std::format(fmt, std::forward<Args>(args)...);
    }
}}

// 全局日志宏（通过 FormatMsg 自动区分单参数/多参数，隐藏 spdlog）
#define DOLAS_LOG_TRACE(...)    ::Dolas::detail::DoLogTrace(::Dolas::LogSystemManager::GetInstance().GetLoggerPtr(), ::Dolas::detail::FormatMsg(__VA_ARGS__))
#define DOLAS_LOG_DEBUG(...)    ::Dolas::detail::DoLogDebug(::Dolas::LogSystemManager::GetInstance().GetLoggerPtr(), ::Dolas::detail::FormatMsg(__VA_ARGS__))
#define DOLAS_LOG_INFO(...)     ::Dolas::detail::DoLogInfo(::Dolas::LogSystemManager::GetInstance().GetLoggerPtr(), ::Dolas::detail::FormatMsg(__VA_ARGS__))
#define DOLAS_LOG_WARN(...)     ::Dolas::detail::DoLogWarn(::Dolas::LogSystemManager::GetInstance().GetLoggerPtr(), ::Dolas::detail::FormatMsg(__VA_ARGS__))
#define DOLAS_LOG_ERROR(...)    ::Dolas::detail::DoLogError(::Dolas::LogSystemManager::GetInstance().GetLoggerPtr(), ::Dolas::detail::FormatMsg(__VA_ARGS__))
#define DOLAS_LOG_CRITICAL(...) ::Dolas::detail::DoLogCritical(::Dolas::LogSystemManager::GetInstance().GetLoggerPtr(), ::Dolas::detail::FormatMsg(__VA_ARGS__))

// 简化版宏（推荐使用这些）
#define LOG_TRACE(...)    DOLAS_LOG_TRACE(__VA_ARGS__)
#define LOG_DEBUG(...)    DOLAS_LOG_DEBUG(__VA_ARGS__)
#define LOG_INFO(...)     DOLAS_LOG_INFO(__VA_ARGS__)
#define LOG_WARN(...)     DOLAS_LOG_WARN(__VA_ARGS__)
#define LOG_ERROR(...)    DOLAS_LOG_ERROR(__VA_ARGS__)
#define LOG_CRITICAL(...) DOLAS_LOG_CRITICAL(__VA_ARGS__)

#endif // DOLAS_LOG_SYSTEM_MANAGER_H
