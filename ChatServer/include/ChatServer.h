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
#include "Configuration.hpp"
#include "Session.h"
#include "Timer.h"

class ChatServer
{
public:
    using HandleProtocolFrameCallback = std::function<void(const Session::Ptr &, const ProtocolFrame::Ptr &)>;
    ChatServer(unsigned short port, size_t threadNum = WORK_THREAD_NUM);
    ~ChatServer();
    void Run();
    void Shutdown();
    inline void SetHandleProtocolFrame(HandleProtocolFrameCallback handleProtocolFrame) // test
    {
        m_handleProtocolFrame = handleProtocolFrame;
    }

    inline void ResetHandleProtocolFrame() // test
    {
        m_handleProtocolFrame = std::bind(&ChatServer::HandleProtocolFrame, this, std::placeholders::_1, std::placeholders::_2);
    }

    inline size_t GetSessionCount() // test
    {
        std::lock_guard<std::mutex> lock(m_sessionMtx);
        return m_sessionMap.size();
    }

private:
    void HandleConnection(const Connection::Ptr &conn);
    void HandleCloseConnection(const Connection::Ptr &conn);
    void HandleProtocolFrame(const std::shared_ptr<Session> &session, const ProtocolFrame::Ptr &frame);
    void HandleCloseSession(const std::shared_ptr<Session> &session);

    AsyncTcpServer m_tcpServer;
    ThreadPool m_servicePool;
    std::atomic<bool> m_isRunning;

    HandleProtocolFrameCallback m_handleProtocolFrame;
    Timer m_timer;

    std::unordered_map<uint32_t, std::weak_ptr<Session>> m_connIdToSession;
    std::mutex m_connIdSessionMtx;
    std::unordered_map<uint32_t, Session::Ptr> m_sessionMap;
    std::mutex m_sessionMtx;
};