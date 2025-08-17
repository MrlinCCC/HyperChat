#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <asio.hpp>
#include <ThreadPool.h>
#include "Connection.h"
#include "Logger.h"
#include "AsyncTcpServer.h"
#include <Protocol.h>
#include <atomic>
#include "Globals.hpp"

class ChatServer
{
public:
    using HandleProtocolRequestCallback = std::function<void(const Connection::Ptr &, const ProtocolRequest::Ptr &)>;
    ChatServer(unsigned short port, size_t threadNum = WORK_THREAD_NUM);
    ~ChatServer();
    void Run();
    void Shutdown();
    void SetHandleProtocolRequest(HandleProtocolRequestCallback handleProtocolRequest);

private:
    void OnProtocolRequest(const std::shared_ptr<Connection> &connection, std::size_t);
    void HandleProtocolRequest(const Connection::Ptr &connection, const ProtocolRequest::Ptr &req);

    AsyncTcpServer m_tcpServer;
    ThreadPool m_servicePool;
    std::atomic<bool> m_isRunning;

    HandleProtocolRequestCallback m_handleProtocolRequest;
};