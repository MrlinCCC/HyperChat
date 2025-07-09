#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <asio.hpp>
#include <ThreadPool.h>
#include "Session.h"
#include "atomic"
#include "Logger.h"
#include "ChatRoom.h"
#include "ProtocolMessage.h"

#define INIT_SESSION_BUCKET_SIZE 1024
#define INIT_THREAD_NUM 4

class ChatServer
{
public:
    ChatServer(unsigned short port, size_t threadNum = INIT_THREAD_NUM);
    ~ChatServer();
    void Run();
    void AsyncAcceptConnection();
    void HandleRequest(const Session::ptr &session, const ProtocolMessage &request);
    void Shutdown();
    size_t GetSessionCount();

private:
    asio::io_context m_ioContext;
    asio::executor_work_guard<asio::io_context::executor_type> m_workGuard;
    asio::ip::tcp::acceptor m_acceptor;

    std::unordered_map<uint32_t, Session::ptr> m_sessions;
    std::mutex m_sessionsMtx;

    ThreadPool m_workThreadPools;
    std::atomic<bool> m_isRunning;
    BinarySemaphore m_allAsyncFinish;
};