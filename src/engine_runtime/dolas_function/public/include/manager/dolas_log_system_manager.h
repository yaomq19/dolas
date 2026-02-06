#ifndef DOLAS_LOG_SYSTEM_MANAGER_H
#define DOLAS_LOG_SYSTEM_MANAGER_H

#include <string>
#include <memory>
#include "spdlog/spdlog.h"
#include "dolas_log.h"

namespace Dolas
{
    class LogSystemManager
    {
    public:
        LogSystemManager();
        ~LogSystemManager();

        bool Initialize();
        bool Clear();

        // 获取日志器
        std::shared_ptr<spdlog::logger> GetLogger() const { return m_logger; }

    private:
        std::shared_ptr<spdlog::logger> m_logger;
        std::string m_log_file_path;
    };
}

#endif // DOLAS_LOG_SYSTEM_MANAGER_H

