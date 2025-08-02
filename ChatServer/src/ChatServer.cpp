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

void ChatServer::HandleProtocolMessage(const std::shared_ptr<Session> &session, const ProtocolRequestMessage::Ptr &ProtocolMsg)
{
    auto requestSession = session->shared_from_this();
    if (m_isRunning)
    {
        m_servicePool.SubmitTask(
            [this](std::shared_ptr<Session> session, ProtocolRequestMessage::Ptr message)
            {
                ProtocolResponseMessage::Ptr responseMsg = ChatService::GetInstance().ExecuteService(session, message);
                m_tcpServer.AsyncWriteMessage(session, responseMsg);
            },
            requestSession, ProtocolMsg);
    }
}