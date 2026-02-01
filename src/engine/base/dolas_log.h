#ifndef DOLAS_LOG_H
#define DOLAS_LOG_H

#include "spdlog/spdlog.h"

// Dolas 日志宏定义
// 这些宏会自动捕获源位置信息（文件名、行号、函数名）

// 基础日志宏（包含源位置信息）
#define DOLAS_LOG_TRACE(...)    SPDLOG_TRACE(__VA_ARGS__)
#define DOLAS_LOG_DEBUG(...)    SPDLOG_DEBUG(__VA_ARGS__)
#define DOLAS_LOG_INFO(...)     SPDLOG_INFO(__VA_ARGS__)
#define DOLAS_LOG_WARN(...)     SPDLOG_WARN(__VA_ARGS__)
#define DOLAS_LOG_ERROR(...)    SPDLOG_ERROR(__VA_ARGS__)
#define DOLAS_LOG_CRITICAL(...) SPDLOG_CRITICAL(__VA_ARGS__)

// 简化版宏（推荐使用这些）
#define LOG_TRACE(...)    SPDLOG_TRACE(__VA_ARGS__)
#define LOG_DEBUG(...)    SPDLOG_DEBUG(__VA_ARGS__)
#define LOG_INFO(...)     SPDLOG_INFO(__VA_ARGS__)
#define LOG_WARN(...)     SPDLOG_WARN(__VA_ARGS__)
#define LOG_ERROR(...)    SPDLOG_ERROR(__VA_ARGS__)
#define LOG_CRITICAL(...) SPDLOG_CRITICAL(__VA_ARGS__)

#endif // DOLAS_LOG_H

