#include "dolas_log_system_manager.h"
#include "base/dolas_paths.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <filesystem>
#include <iostream>
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
    
    bool LogSystemManager::Initialize()
    {
        // 获取当前时间戳
        auto now = std::chrono::system_clock::now();
        auto now_time_t = std::chrono::system_clock::to_time_t(now);
        std::tm local_tm;
        localtime_s(&local_tm, &now_time_t);
            
        // 格式化时间戳为文件名：dolas_2025-10-18_22-47-15.log
        std::ostringstream timestamp_stream;
        timestamp_stream << "dolas_"
                        << std::setfill('0') << std::setw(4) << (local_tm.tm_year + 1900) << "-"
                        << std::setfill('0') << std::setw(2) << (local_tm.tm_mon + 1) << "-"
                        << std::setfill('0') << std::setw(2) << local_tm.tm_mday << "_"
                        << std::setfill('0') << std::setw(2) << local_tm.tm_hour << "-"
                        << std::setfill('0') << std::setw(2) << local_tm.tm_min << "-"
                        << std::setfill('0') << std::setw(2) << local_tm.tm_sec << ".log";
            
        // 获取项目根目录并创建日志文件的完整路径
        std::filesystem::path log_dir = std::filesystem::path(CMAKE_SOURCE_DIR) / "logs";
        std::filesystem::path log_file = log_dir / timestamp_stream.str();
        m_log_file_path = log_file.string();
            
        // 确保日志文件夹存在
        std::filesystem::create_directories(log_dir);
            
        // 配置文件日志：旋转文件，最大10MB，最多保留5个文件
        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            log_file.string(), 1024 * 1024 * 10, 5);
        file_sink->set_level(spdlog::level::trace); // 文件记录所有级别
            
        // 配置控制台日志（带颜色）
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::info); // 控制台只显示info及以上级别
            
        // 创建多sink的logger
        std::vector<spdlog::sink_ptr> sinks {file_sink, console_sink};
        m_logger = std::make_shared<spdlog::logger>("dolas_logger", sinks.begin(), sinks.end());
        m_logger->set_level(spdlog::level::trace); // logger整体级别
        // 格式说明：%s=文件名 %#=行号 %!=函数名 %Y-%m-%d=日期 %H:%M:%S=时间 %e=毫秒 %l=日志级别 %t=线程ID %v=消息内容
        m_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%s:%#] [%!] [thread %t] %v");
            
        // 设置为默认logger
        spdlog::set_default_logger(m_logger);
        spdlog::flush_on(spdlog::level::warn); // 警告及以上级别立即写入文件
            
        SPDLOG_INFO("=== Dolas System Initialize Success ===");
        SPDLOG_INFO("Log file save path: {0}", m_log_file_path);
            
        return true;
    }

    bool LogSystemManager::Clear()
    {
        if (m_logger)
        {
            SPDLOG_INFO("=== Dolas Log System Shutdown ===");
            m_logger->flush();
        }
        
        // 确保所有日志都被写入文件
        spdlog::shutdown();
        
        return true;
    }
}

