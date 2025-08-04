#pragma once
#include "functional"
#include "Connection.h"
#include "ThreadPool.h"
#include <IoContextPool.h>
#include "UncopybleAndUnmovable.h"
#include "Protocol.h"

#define RW_THREAD_NUM 1
#define CONNECTION_BUCKET_SIZE 1024

class AsyncTcpServer : public UncopybleAndUnmovable
{
public:
    using HandleProtocolMessage = std::function<void(const Connection::Ptr &, const ProtocolRequest::Ptr &)>;

    AsyncTcpServer(unsigned short port, size_t rwThreadNum = RW_THREAD_NUM);
    ~AsyncTcpServer();
    void Run();
    void Shutdown();
    void SetHandleProtocolMessage(HandleProtocolMessage handleRequest);
    void AsyncWriteMessage(const Connection::Ptr &conn, const ProtocolResponse::Ptr &message);
    size_t GetConnectionCount();

private:
    void AsyncAcceptConnection();
    void OnMessage(const Connection::Ptr &conn, std::size_t length);

    asio::io_context m_connContext;
    asio::executor_work_guard<asio::io_context::executor_type> m_workGuard;
    asio::ip::tcp::acceptor m_acceptor;
    IoContextPool m_rwContextPool;
    std::unordered_map<uint32_t, Connection::Ptr> m_connections;
    std::mutex m_connsMtx;
    std::atomic<bool> m_isRunning;
    BinarySemaphore m_allAsyncFinish;

    ProtocolCodec m_protocolCodec;
    HandleProtocolMessage m_handleProtocolRequest;
};