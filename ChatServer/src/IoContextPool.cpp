#include "IoContextPool.h"

IoContextPool::IoContextPool(size_t contextNum) : m_contextNum(contextNum),
                                                  m_ioContexts(contextNum), m_threadPool(contextNum), m_nextIoContext(0), m_isRunning(true)
{
    m_workGuards.reserve(m_contextNum);
    for (int i = 0; i < m_contextNum; ++i)
    {
        m_workGuards.emplace_back(asio::make_work_guard(m_ioContexts[i]));
    }
}

void IoContextPool::Run()
{
    for (int i = 0; i < m_contextNum; ++i)
        m_threadPool.SubmitTask([this, i]()
                                { try {
        m_ioContexts[i].run();
    } catch (const std::exception &e) {
        LOG_ERROR("IoContextPool['{}'] run failed: {}",i,e.what());
    } });
}

void IoContextPool::Shutdown()
{
    if (m_isRunning)
    {
        m_isRunning = false;
        for (int i = 0; i < m_contextNum; ++i)
        {
            m_workGuards[i].reset();
        }
        m_threadPool.Shutdown();
    }
}

IoContextPool::~IoContextPool()
{
    if (m_isRunning)
        Shutdown();
}

asio::io_context &IoContextPool::GetIoContext()
{
    if (!m_isRunning)
    {
        throw std::runtime_error("IoContextPool is stopped, cannot dispatch io_context.");
    }

    size_t index = m_nextIoContext.fetch_add(1, std::memory_order_relaxed);
    return m_ioContexts[index % m_contextNum];
}
