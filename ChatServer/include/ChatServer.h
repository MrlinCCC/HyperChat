#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <asio.hpp>
#include <ThreadPool.h>
#include "Connection.h"
#include "atomic"
#include "Logger.h"
#include "AsyncTcpServer.h"
#include <Protocol.h>
#include <atomic>

#define WORK_THREAD_NUM 4

class ChatServer
{
public:
    ChatServer(unsigned short port, size_t threadNum = WORK_THREAD_NUM);
    ~ChatServer();
    void Run();
    void Shutdown();

private:
    void HandleProtocolMessage(const std::shared_ptr<Connection> &connection, const ProtocolRequest::Ptr &ProtocolMsg);

    AsyncTcpServer m_tcpServer;
    ThreadPool m_servicePool;
    std::atomic<bool> m_isRunning;
};