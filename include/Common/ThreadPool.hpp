#pragma once
#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <future>

using Task = std::function<void()>;

class ThreadPool
{
public:
    ThreadPool(const ThreadPool &) = delete;
    ThreadPool &operator=(const ThreadPool &) = delete;
    ThreadPool(ThreadPool &&) = delete;
    ThreadPool &operator=(ThreadPool &&) = delete;
    ThreadPool(size_t threadNum) : m_isRunning(true)
    {
        m_workThreads.reserve(threadNum);
        for (int i = 0; i < threadNum; ++i)
        {
            m_workThreads.emplace_back(&ThreadPool::ProcessTask, this);
        }
    };

    template <typename F, typename... Args>
    auto submitTask(F &&func, Args &&...args)
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

    ~ThreadPool()
    {
        m_isRunning = false;
        m_cv.notify_all();
        for (auto &t : m_workThreads)
        {
            if (t.joinable())
            {
                t.join();
            }
        }
    };

private:
    void ProcessTask()
    {
        while (m_isRunning || !m_taskQueue.empty())
        {
            std::unique_lock<std::mutex> lock(m_mtx);
            m_cv.wait(lock, [this]()
                      { return !m_taskQueue.empty() || !m_isRunning; });
            if (!m_isRunning && m_taskQueue.empty())
            {
                break;
            }
            Task task = m_taskQueue.front();
            m_taskQueue.pop();
            lock.unlock();
            task();
        }
    }

    std::mutex m_mtx;
    std::condition_variable m_cv;
    std::vector<std::thread> m_workThreads;
    std::queue<Task> m_taskQueue;
    std::atomic<bool> m_isRunning;
};