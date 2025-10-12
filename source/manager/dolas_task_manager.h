#ifndef DOLAS_TASK_MANAGER_H
#define DOLAS_TASK_MANAGER_H

#include <future>
#include <functional>
#include "base/dolas_base.h"
#include "ThreadPool/ThreadPool.h"

// 前向声明
class ThreadPool;

namespace Dolas
{
    /**
     * @brief 任务管理器，封装线程池功能
     * 
     * TaskManager 负责管理多线程任务执行，内部封装了 ThreadPool
     * 提供了简洁的接口来提交和管理异步任务
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
         * @return 返回 std::future 对象，可以用来等待任务完成或获取结果
         */
        template<class F, class... Args>
        auto EnqueueTask(F&& f, Args&&... args) 
            -> std::future<typename std::invoke_result<F, Args...>::type>;

        /**
         * @brief 获取线程池中的工作线程数量
         * @return 工作线程数量
         */
        UInt GetWorkerCount() const { return m_worker_count; }

    private:
        ThreadPool* m_thread_pool;
        UInt m_worker_count;
    };

    // 模板函数实现必须在头文件中
    template<class F, class... Args>
    auto TaskManager::EnqueueTask(F&& f, Args&&... args) 
        -> std::future<typename std::invoke_result<F, Args...>::type>
    {
        if (!m_thread_pool)
        {
            // 如果线程池未初始化，返回一个无效的 future
            using return_type = typename std::invoke_result<F, Args...>::type;
            std::promise<return_type> promise;
            promise.set_exception(std::make_exception_ptr(std::runtime_error("TaskManager not initialized")));
            return promise.get_future();
        }
        
        return m_thread_pool->enqueue(std::forward<F>(f), std::forward<Args>(args)...);
    }
}

#endif // DOLAS_TASK_MANAGER_H
