#include "SpinLock.h"

void SpinLock::lock()
{
    while (m_flag.test_and_set(std::memory_order_acquire))
    {
        // 自旋等待
    }
}

void SpinLock::unlock()
{
    m_flag.clear(std::memory_order_release);
}
