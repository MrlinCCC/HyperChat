#include "ThreadPool.h"

ThreadPool::ThreadPool(size_t threadNum) : m_isRunning(true)
{
    m_workThreads.reserve(threadNum);
    for (size_t i = 0; i < threadNum; ++i)
    {
        m_workThreads.emplace_back(&ThreadPool::ProcessTask, this);
    }
}

void ThreadPool::Shutdown()
{
    if (m_isRunning)
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
    }
}

void ThreadPool::ProcessTask()
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

ThreadPool::~ThreadPool()
{
    Shutdown();
}