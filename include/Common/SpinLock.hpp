#pragma once
#include <atomic>

class SpinLock
{
public:
    SpinLock() = default;
    ~SpinLock() = default;
    SpinLock &operator=(const SpinLock &) = delete;
    SpinLock(const SpinLock &) = delete;

    void lock()
    {
        while (m_flag.test_and_set(std::memory_order_acquire))
        {
            ;
        }
    }

    void unlock()
    {
        m_flag.clear(std::memory_order_release);
    }

private:
    std::atomic_flag m_flag = ATOMIC_FLAG_INIT;
};
