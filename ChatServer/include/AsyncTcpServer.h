#pragma once
#include "functional"
#include "Session.h"
#include "ThreadPool.h"
#include <IoContextPool.h>
#include "UncopybleAndUnmovable.h"
#include "Protocol.h"

#define RW_THREAD_NUM 1
#define SESSION_BUCKET_SIZE 1024

class AsyncTcpServer : public UncopybleAndUnmovable
{
public:
    using HandleProtocolMessage = std::function<void(const Session::Ptr &, const ProtocolRequestMessage::Ptr &)>;

    AsyncTcpServer(unsigned short port, size_t rwThreadNum = RW_THREAD_NUM);
    ~AsyncTcpServer();
    void Run();
    void Shutdown();
    void SetHandleProtocolMessage(HandleProtocolMessage handleRequest);
    void AsyncWriteMessage(const Session::Ptr &session, const ProtocolResponseMessage::Ptr &message);
    size_t GetSessionCount();

private:
    void AsyncAcceptConnection();
    void OnMessage(const Session::Ptr &session, std::size_t length);

    asio::io_context m_connContext;
    asio::executor_work_guard<asio::io_context::executor_type> m_workGuard;
    asio::ip::tcp::acceptor m_acceptor;
    IoContextPool m_rwContextPool;
    std::unordered_map<uint32_t, Session::Ptr> m_sessions;
    std::mutex m_sessionsMtx;
    std::atomic<bool> m_isRunning;
    BinarySemaphore m_allAsyncFinish;

    ProtocolCodec m_protocolCodec;
    HandleProtocolMessage m_handleProtocolRequest;
};