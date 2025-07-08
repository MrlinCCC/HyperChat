#include "Semaphore.h"

CountSemaphore::CountSemaphore(size_t init)
{
#ifdef PLATFORM_WINDOWS
    m_handle = CreateSemaphore(nullptr, static_cast<LONG>(init), LONG_MAX, nullptr);
    if (!m_handle)
    {
        throw std::runtime_error("CreateSemaphore failed!");
    }
#else
    if (sem_init(&m_sem, 0, init) != 0)
    {
        throw std::runtime_error("sem_init failed!");
    }
#endif
}

CountSemaphore::~CountSemaphore()
{
#ifdef PLATFORM_WINDOWS
    if (m_handle)
    {
        CloseHandle(m_handle);
        m_handle = nullptr;
    }
#else
    sem_destroy(&m_sem);
#endif
}

void CountSemaphore::Acquire()
{
#ifdef PLATFORM_WINDOWS
    DWORD result = WaitForSingleObject(m_handle, INFINITE);
    if (result != WAIT_OBJECT_0)
    {
        throw std::runtime_error("WaitForSingleObject failed!");
    }
#else
    while (sem_wait(&m_sem) == -1 && errno == EINTR)
    {
    }
#endif
}

bool CountSemaphore::Acquire_for(std::chrono::seconds timeout)
{
#ifdef PLATFORM_WINDOWS
    DWORD result = WaitForSingleObject(m_handle, static_cast<DWORD>(timeout.count() * 1000));
    if (result == WAIT_OBJECT_0)
        return true;
    if (result == WAIT_TIMEOUT)
        return false;
    throw std::runtime_error("WaitForSingleObject failed!");
#else
    timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += timeout.count();
    while (true)
    {
        if (sem_timedwait(&m_sem, &ts) == 0)
            return true;
        if (errno == EINTR)
            continue;
        if (errno == ETIMEDOUT)
            return false;
        throw std::runtime_error("sem_timedwait failed!");
    }
#endif
}

void CountSemaphore::Release()
{
#ifdef PLATFORM_WINDOWS
    if (!ReleaseSemaphore(m_handle, 1, nullptr))
    {
        throw std::runtime_error("ReleaseSemaphore failed!");
    }
#else
    if (sem_post(&m_sem) != 0)
    {
        throw std::runtime_error("sem_post failed!");
    }
#endif
}

BinarySemaphore::BinarySemaphore(size_t init)
    : CountSemaphore(init ? 1 : 0)
{
}
