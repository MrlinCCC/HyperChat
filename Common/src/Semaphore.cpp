#include "Semaphore.h"

CountSemaphore::CountSemaphore(size_t init) : m_count(init)
{
}

void CountSemaphore::acquire()
{
    std::unique_lock<std::mutex> lock(m_mtx);
    m_cv.wait(lock, [this]()
              { return m_count > 0; });
    m_count--;
}

bool CountSemaphore::acquire_for(std::chrono::seconds timeout)
{
    std::unique_lock<std::mutex> lock(m_mtx);
    bool acquire = m_cv.wait_for(lock, timeout, [this]()
                                 { return m_count > 0; });
    if (acquire)
        m_count--;
    return acquire;
}

void CountSemaphore::release()
{
    std::lock_guard<std::mutex> lock(m_mtx);
    m_count++;
    m_cv.notify_one();
}

void CountSemaphore::reset(size_t init)
{
    std::lock_guard<std::mutex> lock(m_mtx);
    m_count = init;
}

BinarySemaphore::BinarySemaphore(size_t init) : m_count(init > 0 ? 1 : 0)
{
}

void BinarySemaphore::acquire()
{
    std::unique_lock<std::mutex> lock(m_mtx);
    m_cv.wait(lock, [this]
              { return m_count > 0; });
    m_count = 0;
}

bool BinarySemaphore::acquire_for(std::chrono::seconds timeout)
{
    std::unique_lock<std::mutex> lock(m_mtx);
    if (!m_cv.wait_for(lock, timeout, [this]
                       { return m_count > 0; }))
        return false;
    m_count = 0;
    return true;
}

void BinarySemaphore::release()
{
    std::lock_guard<std::mutex> lock(m_mtx);
    if (m_count == 0)
    {
        m_count = 1;
        m_cv.notify_one();
    }
}

void BinarySemaphore::reset(size_t init)
{
    std::lock_guard<std::mutex> lock(m_mtx);
    m_count = (init > 0 ? 1 : 0);
}