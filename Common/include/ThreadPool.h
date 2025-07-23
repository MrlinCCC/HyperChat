#pragma once
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <future>
#include <vector>
#include "UncopybleAndUnmovable.h"

using Task = std::function<void()>;

class ThreadPool : public UncopybleAndUnmovable
{
public:
    explicit ThreadPool(size_t threadNum);

    template <typename F, typename... Args>
    auto SubmitTask(F &&func, Args &&...args)
        -> std::future<std::invoke_result_t<F, Args...>>
    {
        using ReturnType = std::invoke_result_t<F, Args...>;
        auto taskPtr = std::make_shared<std::packaged_task<ReturnType()>>(
            std::bind(std::forward<F>(func), std::forward<Args>(args)...));
        std::future<ReturnType> future = taskPtr->get_future();
        {
            std::lock_guard<std::mutex> lock(m_mtx);
            m_taskQueue.push([taskPtr]()
                             { (*taskPtr)(); });
        }
        m_cv.notify_one();
        return future;
    }

    void Shutdown();

    ~ThreadPool();

private:
    void ProcessTask();

    std::mutex m_mtx;
    std::condition_variable m_cv;
    std::vector<std::thread> m_workThreads;
    std::queue<Task> m_taskQueue;
    std::atomic<bool> m_isRunning;
};
