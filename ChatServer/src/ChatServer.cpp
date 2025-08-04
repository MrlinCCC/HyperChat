#include "ChatServer.h"
#include "ChatService.h"

ChatServer::ChatServer(unsigned short port, size_t threadNum)
    : m_tcpServer(port),
      m_servicePool(threadNum),
      m_isRunning(false)
{
    auto handleProtocolMessage = std::bind(&ChatServer::HandleProtocolMessage, this, std::placeholders::_1, std::placeholders::_2);
    m_tcpServer.SetHandleProtocolMessage(handleProtocolMessage);
}

ChatServer::~ChatServer()
{
    if (m_isRunning)
        Shutdown();
}

void ChatServer::Run()
{
    m_isRunning = true;
    m_tcpServer.Run();
}

void ChatServer::Shutdown()
{
    if (m_isRunning)
    {
        m_isRunning = false;
        m_tcpServer.Shutdown();
        m_servicePool.Shutdown();
    }
}

void ChatServer::HandleProtocolMessage(const std::shared_ptr<Connection> &connection, const ProtocolRequest::Ptr &ProtocolMsg)
{
    auto conn = connection->shared_from_this();
    if (m_isRunning)
    {
        m_servicePool.SubmitTask(
            [this](std::shared_ptr<Connection> connection, ProtocolRequest::Ptr message)
            {
                ProtocolResponse::Ptr responseMsg = ChatService::GetInstance().ExecuteService(connection, message);
                m_tcpServer.AsyncWriteMessage(connection, responseMsg);
            },
            conn, ProtocolMsg);
    }
}