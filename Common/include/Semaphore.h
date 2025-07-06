#pragma once

#include <condition_variable>
#include <chrono>
#include <mutex>

class CountSemaphore
{
public:
    explicit CountSemaphore(size_t init = 0);

    void acquire();

    bool acquire_for(std::chrono::seconds timeout);

    void release();

    void reset(size_t init = 0);

private:
    std::mutex m_mtx;
    std::condition_variable m_cv;
    size_t m_count;
};

class BinarySemaphore
{
public:
    explicit BinarySemaphore(size_t init = 0);

    void acquire();

    bool acquire_for(std::chrono::seconds timeout);

    void release();

    void reset(size_t init = 0);

private:
    std::mutex m_mtx;
    std::condition_variable m_cv;
    size_t m_count;
};