#pragma once
#include <vector>
#include <asio.hpp>
#include <ThreadPool.h>
#include <mutex>
#include <Logger.h>
#include "UncopybleAndUnmovable.h"

class IoContextPool : public UncopybleAndUnmovable
{
public:
    explicit IoContextPool(size_t contextNum);
    ~IoContextPool();
    void Run();
    void Shutdown();
    asio::io_context &GetIoContext();

private:
    size_t m_contextNum;
    std::vector<asio::io_context> m_ioContexts;
    std::vector<asio::executor_work_guard<asio::io_context::executor_type>> m_workGuards;
    ThreadPool m_threadPool;
    std::atomic<size_t> m_nextIoContext;
    std::atomic<bool> m_isRunning;
};
