#include "dolas_log_system_manager.h"
#include <spdlog/spdlog.h>
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <filesystem>
#include <iostream>
#include <Windows.h>
#include <vector>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace Dolas
{
    LogSystemManager::LogSystemManager()
        : m_logger(nullptr)
        , m_log_file_path("")
    {
    }

    LogSystemManager::~LogSystemManager()
    {
    }

    // 单例模式实现
    LogSystemManager& LogSystemManager::GetInstance()
    {
        static LogSystemManager instance;
        return instance;
    }

    // 获取 logger 指针（作为 void* 返回以隐藏类型）
    void* LogSystemManager::GetLoggerPtr() const
    {
        return static_cast<void*>(m_logger.get());
    }

    // 内部辅助函数实现（在这里才暴露 spdlog 的细节）
    namespace detail
    {
        void DoLogTrace(void* logger, const std::string& msg)
        {
            if (logger)
            {
                static_cast<spdlog::logger*>(logger)->trace(msg);
            }
        }

        void DoLogDebug(void* logger, const std::string& msg)
        {
            if (logger)
            {
                static_cast<spdlog::logger*>(logger)->debug(msg);
            }
        }

        void DoLogInfo(void* logger, const std::string& msg)
        {
            if (logger)
            {
                static_cast<spdlog::logger*>(logger)->info(msg);
            }
        }

        void DoLogWarn(void* logger, const std::string& msg)
        {
            if (logger)
            {
                static_cast<spdlog::logger*>(logger)->warn(msg);
            }
        }

        void DoLogError(void* logger, const std::string& msg)
        {
            if (logger)
            {
                static_cast<spdlog::logger*>(logger)->error(msg);
            }
        }

        void DoLogCritical(void* logger, const std::string& msg)
        {
            if (logger)
            {
                static_cast<spdlog::logger*>(logger)->critical(msg);
            }
        }
    }
    
    bool LogSystemManager::Initialize()
    {
        // Get current timestamp
        auto now = std::chrono::system_clock::now();
        auto now_time_t = std::chrono::system_clock::to_time_t(now);
        std::tm local_tm;
        localtime_s(&local_tm, &now_time_t);
            
        // Format timestamp as filename: dolas_2025-10-18_22-47-15.log
        std::ostringstream timestamp_stream;
        timestamp_stream << "dolas_"
                        << std::setfill('0') << std::setw(4) << (local_tm.tm_year + 1900) << "-"
                        << std::setfill('0') << std::setw(2) << (local_tm.tm_mon + 1) << "-"
                        << std::setfill('0') << std::setw(2) << local_tm.tm_mday << "_"
                        << std::setfill('0') << std::setw(2) << local_tm.tm_hour << "-"
                        << std::setfill('0') << std::setw(2) << local_tm.tm_min << "-"
                        << std::setfill('0') << std::setw(2) << local_tm.tm_sec << ".log";
            
        // Get project root directory and create full path for log file
        // Get executable directory and create full path for log file (use `log/` in the same folder as the exe)
        char exe_path[MAX_PATH] = {0};
        DWORD len = GetModuleFileNameA(nullptr, exe_path, MAX_PATH);
        std::filesystem::path exe_file;
        if (len > 0)
        {
            exe_file = std::filesystem::path(std::string(exe_path, static_cast<size_t>(len)));
        }
        else
        {
            exe_file = std::filesystem::current_path(); // fallback to current path
        }
        std::filesystem::path log_dir = exe_file.parent_path() / "log";
        std::filesystem::path log_file = log_dir / timestamp_stream.str();
        m_log_file_path = log_file.string();

        // Ensure log directory exists
        std::error_code ec;
        std::filesystem::create_directories(log_dir, ec);
        if (ec)
        {
            std::cerr << "Failed to create log directory: " << ec.message() << std::endl;
        }
            
        // Configure file sink: rotating file, max 10MB, keep at most 5 files
        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            log_file.string(), 1024 * 1024 * 10, 5);
        file_sink->set_level(spdlog::level::trace); // File logs all levels
            
        // Configure console sink (with colors)
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::info); // Console only shows info and above
            
        // Create multi-sink logger
        std::vector<spdlog::sink_ptr> sinks {file_sink, console_sink};
        m_logger = std::make_shared<spdlog::logger>("dolas_logger", sinks.begin(), sinks.end());
        m_logger->set_level(spdlog::level::trace); // Overall logger level
        // Format: %s=filename %#=line %!=function %Y-%m-%d=date %H:%M:%S=time %e=milliseconds %l=level %t=thread %v=message
        m_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%s:%#] [%!] [thread %t] %v");
            
        // Set as default logger
        spdlog::set_default_logger(m_logger);
        spdlog::flush_on(spdlog::level::warn); // Flush immediately for warnings and above
            
        m_logger->info("=== Dolas System Initialize Success ===");
        m_logger->info("Log file save path: {0}", m_log_file_path);
            
        return true;
    }

    bool LogSystemManager::Clear()
    {
        if (m_logger)
        {
            m_logger->info("=== Dolas Log System Shutdown ===");
            m_logger->flush();
        }
        
        // Ensure all logs are written to file
        spdlog::shutdown();
        
        return true;
    }
}

