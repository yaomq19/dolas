#include "base/dolas_paths.h"
#include "core/dolas_engine.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <vector>
#include <filesystem>
#include <iostream>

int main()
{
	// 获取项目根目录并创建日志文件的完整路径
	std::string project_root = Dolas::PathUtils::GetContentDir(); // 获取 content 目录的父目录
	std::filesystem::path log_dir = std::filesystem::path(CMAKE_SOURCE_DIR) / "logs";
	std::filesystem::path log_file = log_dir / "dolas.log";
	
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
	auto logger = std::make_shared<spdlog::logger>("dolas_logger", sinks.begin(), sinks.end());
	logger->set_level(spdlog::level::trace); // logger整体级别
	logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [thread %t] %v"); // 自定义格式
	
	// 设置为默认logger
	spdlog::set_default_logger(logger);
	spdlog::flush_on(spdlog::level::warn); // 警告及以上级别立即写入文件
	
	spdlog::info("Welcome to spdlog!");
	spdlog::info("日志文件保存位置: {}", log_file.string());

	if (Dolas::g_dolas_engine.Initialize())
    {
        // Dolas::g_dolas_engine.Run();
        Dolas::g_dolas_engine.Run();
    }
    
    // 程序退出前清理所有资源
    Dolas::g_dolas_engine.Clear();
    
    // 确保所有日志都被写入文件
    spdlog::shutdown();
    
    return 0;
}