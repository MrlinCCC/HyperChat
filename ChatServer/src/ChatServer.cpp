#include "ChatServer.h"
#include "ChatService.h"

ChatServer::ChatServer(unsigned short port, size_t threadNum)
    : m_ioContext(),
      m_workGuard(asio::make_work_guard(m_ioContext)),
      m_acceptor(m_ioContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)),
      m_workThreadPools(threadNum),
      m_isRunning(true)
{
    m_sessions.reserve(INIT_SESSION_BUCKET_SIZE);
    m_acceptor.listen(SERVER_BACKLOG);
}

ChatServer::~ChatServer()
{
    if (m_isRunning)
        Shutdown();
}

void ChatServer::AsyncAcceptConnection()
{
    m_acceptor.async_accept([this](const asio::error_code &ec, asio::ip::tcp::socket socket)
                            {
        if (ec == asio::error::operation_aborted) {
            return;
        }
        if(ec){
            LOG_ERROR(ec.message());
            return;
        }
        auto newSessionPtr = std::make_shared<Session>(std::move(socket), 
            std::bind(&ChatServer::HandleRequest, this, std::placeholders::_1, std::placeholders::_2));
        {
        std::lock_guard<std::mutex> lock(m_sessionsMtx);
        m_sessions.insert(std::make_pair(newSessionPtr->getSessionId(),newSessionPtr));
        }
        newSessionPtr->AsyncReadMessage();
        AsyncAcceptConnection(); });
}

void ChatServer::Run()
{
    AsyncAcceptConnection();
    m_ioContext.run();
    m_allAsyncFinish.Release();
}

void ChatServer::HandleRequest(const std::shared_ptr<Session> &session, const ProtocolMessage &message)
{
    auto requestSession = session->shared_from_this();
    if (m_isRunning)
    {
        m_workThreadPools.SubmitTask(
            [this, requestSession](ProtocolMessage message)
            {
                ProtocolMessage response = ChatService::GetInstance().ExecuteService(message);
                requestSession->AsyncWriteMessage(response);
            },
            message);
    }
}

void ChatServer::Shutdown()
{
    m_isRunning = false;
    if (m_acceptor.is_open())
    {
        m_acceptor.cancel();
        m_acceptor.close();
    }
    m_workThreadPools.Shutdown();
    for (auto &[id, session] : m_sessions)
    {
        session->CloseSession();
    }
    m_workGuard.reset();
    m_allAsyncFinish.Acquire();
}

size_t ChatServer::GetSessionCount()
{
    std::lock_guard<std::mutex> lock(m_sessionsMtx);
    return m_sessions.size();
}