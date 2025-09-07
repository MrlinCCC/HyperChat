#include "Timer.h"

TimeHolder::TimeHolder() : m_t(std::chrono::steady_clock::now()) {}

void TimeHolder::Hold(std::chrono::milliseconds msec)
{
    std::this_thread::sleep_until(msec + m_t);
}

void TimeHolder::Reset()
{
    m_t = std::chrono::steady_clock::now();
}

Timer::Timer(long long tickMs, long long wheelSize, long long workThread) : m_isRunning(false), m_workPool(workThread)
{
    p_headWheel = std::make_unique<TimerWheel>(tickMs, wheelSize);
}

Timer::~Timer()
{
    if (m_isRunning)
        Shutdown();
}

void Timer::Start()
{
    if (m_isRunning)
        return;
    m_isRunning = true;
    m_thread = std::thread([this]()
                           {
            TimeHolder holder;
            uint32_t times = 0;
            while (m_isRunning)
            {
                if (times == 0) {  
                    holder.Reset();
                    times = 1;
                }
                Tick();
                holder.Hold(std::chrono::milliseconds(p_headWheel->m_tickMs*times++));

            } });
}

void Timer::RunAt(std::chrono::steady_clock::time_point time, TimerTask::TimerCallback cb)
{
    TimerTask task;
    task.m_cb = cb;
    task.m_expireTime = time;
    AddTask(task);
}

void Timer::RunAfter(long long delayMs, TimerTask::TimerCallback cb)
{
    TimerTask task;
    task.m_cb = cb;
    task.m_expireTime = std::chrono::steady_clock::now() + std::chrono::milliseconds(delayMs);
    AddTask(task);
}

void Timer::RunEvery(long long intervalMs, TimerTask::TimerCallback cb)
{
    TimerTask task;
    task.m_cb = cb;
    task.m_expireTime = std::chrono::steady_clock::now() + std::chrono::milliseconds(intervalMs);
    task.m_isEvery = true;
    task.m_intervalMs = intervalMs;
    AddTask(task);
}

void Timer::Shutdown()
{
    m_isRunning = false;
    if (m_thread.joinable())
    {
        m_thread.join();
    }
    m_workPool.Shutdown();
}

void Timer::AddTask(std::shared_ptr<TimerWheel> wheel, const TimerTask &task)
{
    auto now = std::chrono::steady_clock::now();
    auto delay = std::chrono::duration_cast<std::chrono::milliseconds>(
                     task.m_expireTime - now)
                     .count();
    if (delay <= 0)
    {
        m_workPool.SubmitTask(task.m_cb);
        if (task.m_isEvery)
        {
            TimerTask nextTask = task;
            nextTask.m_expireTime += std::chrono::milliseconds(task.m_intervalMs);
            AddTask(nextTask);
        }
        return;
    }
    long long wheelRange = wheel->m_tickMs * wheel->m_wheelSize;
    if (delay < wheelRange)
    {
        long long ticks = 0;
        if (wheel != p_headWheel)
        {
            ticks = (delay - wheel->m_tickMs) / wheel->m_tickMs;
        }
        else
        {
            ticks = delay / wheel->m_tickMs;
        }
        long long index = (wheel->m_currentIndex + ticks) % wheel->m_wheelSize;
        std::lock_guard<std::mutex> lock(wheel->m_slotMutexes[index]);
        wheel->m_slots[index].push_back(task);
    }
    else
    {
        if (!wheel->p_next)
        {
            // 动态扩容
            wheel->p_next = std::make_shared<TimerWheel>(wheelRange, wheel->m_wheelSize);
        }
        AddTask(wheel->p_next, task);
    }
}

void Timer::Cascade(std::shared_ptr<TimerWheel> wheel)
{

    long long curIndex = wheel->m_currentIndex;
    wheel->m_currentIndex = (wheel->m_currentIndex + 1) % wheel->m_wheelSize;
    std::list<TimerTask> slot;
    {
        std::lock_guard<std::mutex> lock(wheel->m_slotMutexes[curIndex]);
        slot = wheel->m_slots[curIndex];
    }

    for (auto it = slot.begin(); it != slot.end();)
    {
        AddTask(*it);
        it = slot.erase(it);
    }

    if (wheel->m_currentIndex == 0 && wheel->p_next)
    {
        Cascade(wheel->p_next);
    }
}

void Timer::AddTask(const TimerTask &task)
{
    AddTask(p_headWheel, task);
}

void Timer::Tick()
{
    {
        long long curIndex = p_headWheel->m_currentIndex;
        p_headWheel->m_currentIndex = (p_headWheel->m_currentIndex + 1) % p_headWheel->m_wheelSize;

        std::list<TimerTask> slot;
        {
            std::lock_guard<std::mutex> lock(p_headWheel->m_slotMutexes[curIndex]);
            slot = p_headWheel->m_slots[curIndex];
        }

        auto now = std::chrono::steady_clock::now();
        for (auto it = slot.begin(); it != slot.end();)
        {
            if (now >= it->m_expireTime)
            {
                m_workPool.SubmitTask(it->m_cb);
                if (it->m_isEvery)
                {
                    TimerTask nextTask = *it;
                    nextTask.m_expireTime += std::chrono::milliseconds(it->m_intervalMs);
                    AddTask(nextTask);
                }
            }
            else
            {
                AddTask(*it);
            }
            it = slot.erase(it);
        }
    }

    if (p_headWheel->m_currentIndex == 0 && p_headWheel->p_next)
    {
        Cascade(p_headWheel->p_next);
    }
}