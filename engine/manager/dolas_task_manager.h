#ifndef DOLAS_TASK_MANAGER_H
#define DOLAS_TASK_MANAGER_H

#include <future>
#include <functional>
#include <unordered_map>
#include <mutex>
#include <memory>
#include <chrono>
#include "base/dolas_base.h"
#include "core/ThreadPool.h"

// 前向声明
class ThreadPool;

namespace Dolas
{
    // 任务 GUID 类型定义
    using TaskGUID = ULong;
    
    // 抽象基类，用于统一管理不同类型的 future
    class TaskFutureBase
    {
    public:
        virtual ~TaskFutureBase() = default;
        virtual void Wait() = 0;
        virtual bool IsReady() = 0;
    };
    
    // 模板类，用于包装具体类型的 future
    template<typename T>
    class TaskFuture : public TaskFutureBase
    {
    public:
        TaskFuture(std::future<T>&& future) : m_future(std::move(future)) {}
        
        void Wait() override
        {
            m_future.wait();
        }
        
        bool IsReady() override
        {
            return m_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
        }
        
        // 获取结果（仅供内部使用）
        T Get()
        {
            return m_future.get();
        }
        
    private:
        std::future<T> m_future;
    };
    
    /**
     * @brief 任务管理器，封装线程池功能
     * 
     * TaskManager 负责管理多线程任务执行，内部封装了 ThreadPool
     * 提供了简洁的接口来提交和管理异步任务，使用 GUID 来管理任务
     */
    class TaskManager
    {
    public:
        TaskManager();
        ~TaskManager();

        bool Initialize();
        bool Clear();

        /**
         * @brief 提交一个任务到线程池执行
         * @param f 要执行的函数
         * @param args 函数参数
         * @return 返回任务的 GUID，可以用来等待任务完成
         */
        template<class F, class... Args>
        TaskGUID EnqueueTask(F&& f, Args&&... args);
        
        /**
         * @brief 等待指定 GUID 的任务完成
         * @param task_guid 任务的 GUID
         * @return 如果任务存在且等待成功返回 true，否则返回 false
         */
        bool WaitForTask(TaskGUID task_guid);
        
        /**
         * @brief 检查指定 GUID 的任务是否已完成
         * @param task_guid 任务的 GUID
         * @return 如果任务已完成返回 true，如果任务不存在或未完成返回 false
         */
        bool IsTaskComplete(TaskGUID task_guid);

        /**
         * @brief 获取线程池中的工作线程数量
         * @return 工作线程数量
         */
        UInt GetWorkerCount() const { return m_worker_count; }

    private:
        ThreadPool* m_thread_pool;
        UInt m_worker_count;
        
        // GUID 和 Future 的映射管理
        std::unordered_map<TaskGUID, std::unique_ptr<TaskFutureBase>> m_task_futures;
        std::mutex m_futures_mutex;
        TaskGUID m_next_guid;
        
        // 生成新的任务 GUID
        TaskGUID GenerateTaskGUID();
        
        // 清理已完成的任务
        void CleanupCompletedTasks();
    };

    // 模板函数实现必须在头文件中
    template<class F, class... Args>
    TaskGUID TaskManager::EnqueueTask(F&& f, Args&&... args)
    {
        if (!m_thread_pool)
        {
            // 如果线程池未初始化，返回无效的 GUID (0)
            return 0;
        }
        
        // 生成新的 GUID
        TaskGUID guid = GenerateTaskGUID();
        
        // 提交任务到线程池
        auto future = m_thread_pool->enqueue(std::forward<F>(f), std::forward<Args>(args)...);
        
        // 创建 TaskFuture 包装器
        using return_type = typename std::invoke_result<F, Args...>::type;
        auto task_future = std::make_unique<TaskFuture<return_type>>(std::move(future));
        
        // 存储到映射表中
        {
            std::lock_guard<std::mutex> lock(m_futures_mutex);
            m_task_futures[guid] = std::move(task_future);
        }
        
        return guid;
    }
}

#endif // DOLAS_TASK_MANAGER_H
