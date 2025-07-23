#include "AsyncTcpServer.h"

AsyncTcpServer::AsyncTcpServer(unsigned short port, size_t rwThreadNum)
    : m_connContext(),
      m_workGuard(asio::make_work_guard(m_connContext)),
      m_acceptor(m_connContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)),
      m_rwContextPool(rwThreadNum),
      m_isRunning(true)
{
    m_sessions.reserve(SESSION_BUCKET_SIZE);
}

AsyncTcpServer::~AsyncTcpServer()
{
    if (m_isRunning)
        Shutdown();
}

void AsyncTcpServer::Run()
{
    AsyncAcceptConnection();
    auto endpoint = m_acceptor.local_endpoint();
    LOG_INFO("TcpServer is running! Listening on {}:{}!", endpoint.address().to_string(), endpoint.port());
    m_rwContextPool.Run();
    m_connContext.run();
    m_allAsyncFinish.Release();
}

void AsyncTcpServer::Shutdown()
{
    if (m_isRunning)
    {
        m_isRunning = false;
        if (m_acceptor.is_open())
        {
            m_acceptor.cancel();
            m_acceptor.close();
        }
        m_rwContextPool.Shutdown();
        std::lock_guard<std::mutex> lock(m_sessionsMtx);
        for (auto &[id, session] : m_sessions)
        {
            session->CloseSession();
        }
        m_workGuard.reset();
        m_allAsyncFinish.Acquire();
        LOG_INFO("TcpServer shutdown!");
    }
}

void AsyncTcpServer::AsyncAcceptConnection()
{
    auto socket = std::make_shared<asio::ip::tcp::socket>(m_connContext);
    m_acceptor.async_accept(*socket, [this, socket](const asio::error_code &ec) mutable
                            {
        if (ec == asio::error::operation_aborted) {
            return;
        }
        if(ec){
            LOG_ERROR(ec.message());
            return;
        }
        auto &ioContext = m_rwContextPool.GetIoContext();
        asio::ip::tcp::socket newSocket(ioContext);
        newSocket.assign(socket->local_endpoint().protocol(), socket->release());

        auto newSessionPtr = std::make_shared<Session>(std::move(newSocket));
        auto onMessage = std::bind(&AsyncTcpServer::OnMessage,this,std::placeholders::_1,std::placeholders::_2);
        newSessionPtr->SetMessageCallback(onMessage);
        {
            std::lock_guard<std::mutex> lock(m_sessionsMtx);
            m_sessions.insert(std::make_pair(newSessionPtr->GetSessionId(),newSessionPtr));
        }
        newSessionPtr->AsyncReadMessage();
        AsyncAcceptConnection(); });
}

void AsyncTcpServer::OnMessage(const Session::Ptr &session, std::size_t length)
{
    auto &buffer = session->GetReadBuf();
    auto protocolMsgs = m_protocolCodec.UnpackProtocolMessage(buffer);
    if (!m_handleProtocolMessage)
    {
        LOG_WARN("HandleProtocolRequest is nullptr!");
        return;
    }
    for (const auto &protocolMsg : protocolMsgs)
    {
        m_handleProtocolMessage(session, protocolMsg);
    }
}

void AsyncTcpServer::AsyncWriteMessage(const Session::Ptr &session, const ProtocolMessage::Ptr &message)
{
    if (!session || !message)
    {
        LOG_ERROR("AsyncWrite error : session or message is nullptr!");
        return;
    }
    session->AsyncWriteMessage(m_protocolCodec.PackProtocolMessage(message));
}

void AsyncTcpServer::SetHandleProtocolMessage(HandleProtocolMessage handleRequest)
{
    if (handleRequest)
    {
        m_handleProtocolMessage = handleRequest;
    }
}

size_t AsyncTcpServer::GetSessionCount()
{
    std::lock_guard<std::mutex> lock(m_sessionsMtx);
    return m_sessions.size();
}