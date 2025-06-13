#pragma once

#include <condition_variable>
#include <chrono>

class Semaphore
{
public:
    explicit Semaphore(uint32_t init = 0) : m_count(init) {}

    void acquire()
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        m_cv.wait(lock, [this]()
                  { return m_count > 0; });
        m_count--;
    }

    bool acquire_for(std::chrono::microseconds timeout)
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        bool acquire = m_cv.wait_for(lock, timeout, [this]()
                                     { return m_count > 0; });
        if (acquire)
            m_count--;

        return acquire;
    }

    void release()
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        m_count++;
        m_cv.notify_one();
    }

private:
    std::mutex m_mtx;
    std::condition_variable m_cv;
    uint32_t m_count;
};