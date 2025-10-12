#include "dolas_task_manager.h"

namespace Dolas
{
    TaskManager::TaskManager()
        : m_thread_pool(nullptr)
        , m_worker_count(4) // 默认使用4个工作线程
    {
    }

    TaskManager::~TaskManager()
    {
        Clear();
    }

    bool TaskManager::Initialize()
    {
        if (m_thread_pool)
        {
            // 已经初始化过了
            return true;
        }

        try
        {
            m_thread_pool = DOLAS_NEW(ThreadPool, m_worker_count);
            return true;
        }
        catch (const std::exception& e)
        {
            // 记录错误日志（如果有日志系统的话）
            // DOLAS_LOG_ERROR("Failed to initialize TaskManager: %s", e.what());
            return false;
        }
    }

    bool TaskManager::Clear()
    {
        if (m_thread_pool)
        {
            DOLAS_DELETE(m_thread_pool);
            m_thread_pool = nullptr;
        }
        return true;
    }
}
