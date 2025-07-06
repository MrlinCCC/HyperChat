#pragma once
#include <atomic>

class SpinLock
{
public:
    SpinLock() = default;
    ~SpinLock() = default;

    SpinLock(const SpinLock &) = delete;
    SpinLock &operator=(const SpinLock &) = delete;

    void lock();
    void unlock();

private:
    std::atomic_flag m_flag = ATOMIC_FLAG_INIT;
};
