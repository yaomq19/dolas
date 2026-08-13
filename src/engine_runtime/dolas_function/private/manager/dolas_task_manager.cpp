#include "manager/dolas_task_manager.h"
#include <chrono>

namespace Dolas
{
    TaskManager::TaskManager()
        : m_thread_pool(nullptr)
        , m_worker_count(4) // 默认使用4个工作线程
        , m_next_guid(1) // GUID 从 1 开始，0 表示无效
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
        // 清理所有待处理的任务
        {
            std::lock_guard<std::mutex> lock(m_futures_mutex);
            m_task_futures.clear();
        }
        
        if (m_thread_pool)
        {
            DOLAS_DELETE(m_thread_pool);
            m_thread_pool = nullptr;
        }
        return true;
    }
    
    TaskGUID TaskManager::GenerateTaskGUID()
    {
        std::lock_guard<std::mutex> lock(m_futures_mutex);
        return m_next_guid++;
    }
    
    bool TaskManager::WaitForTask(TaskGUID task_guid)
    {
        if (task_guid == 0)
        {
            return false; // 无效的 GUID
        }
        
        std::unique_lock<std::mutex> lock(m_futures_mutex);
        auto it = m_task_futures.find(task_guid);
        if (it == m_task_futures.end())
        {
            return false; // 任务不存在
        }
        
        // 获取 TaskFutureBase 指针并释放锁
        auto& task_future = it->second;
        lock.unlock();
        
        try
        {
            // 等待任务完成
            task_future->Wait();
            
            // 任务完成后，从映射中移除
            lock.lock();
            m_task_futures.erase(task_guid);
            return true;
        }
        catch (...)
        {
            // 异常处理
            return false;
        }
    }
    
    bool TaskManager::IsTaskComplete(TaskGUID task_guid)
    {
        if (task_guid == 0)
        {
            return false; // 无效的 GUID
        }
        
        std::lock_guard<std::mutex> lock(m_futures_mutex);
        auto it = m_task_futures.find(task_guid);
        if (it == m_task_futures.end())
        {
            return true; // 任务不存在，可能已经完成并被清理
        }
        
        return it->second->IsReady();
    }
    
    void TaskManager::CleanupCompletedTasks()
    {
        std::lock_guard<std::mutex> lock(m_futures_mutex);
        
        auto it = m_task_futures.begin();
        while (it != m_task_futures.end())
        {
            if (it->second->IsReady())
            {
                it = m_task_futures.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
}
