#pragma once
#include <stdexcept>
#include <chrono>

#if defined(_WIN32) || defined(_WIN64)
#define PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#define PLATFORM_POSIX
#include <semaphore.h>
#include <ctime>
#include <errno.h>
#endif

class CountSemaphore
{
public:
    CountSemaphore(size_t init = 0);

    ~CountSemaphore();

    CountSemaphore(const CountSemaphore &) = delete;
    CountSemaphore &operator=(const CountSemaphore &) = delete;

    void Acquire();

    bool Acquire_for(std::chrono::seconds timeout);

    void Release();

private:
#ifdef PLATFORM_WINDOWS
    HANDLE m_handle = nullptr;
#else
    sem_t m_sem;
#endif
};

class BinarySemaphore : public CountSemaphore
{
public:
    BinarySemaphore(size_t init = 0);
};
