#include "chatclient.h"

ChatClient::ChatClient(const Host &server)
    : m_session(asio::ip::tcp::socket(m_ioContext)), m_workGuard(asio::make_work_guard(m_ioContext)), m_serverHost(server), m_isConnected(false)
{
    auto onResponse = std::bind(&ChatClient::OnResponse, this, std::placeholders::_1, std::placeholders::_2);
    m_session.SetMessageCallback(onResponse);
}

ChatClient::~ChatClient()
{
    if (m_isConnected)
    {
        CloseConnection();
    }
    m_workGuard.reset();
}

void ChatClient::ConnectServer()
{
    asio::ip::tcp::resolver resolver(m_ioContext);
    asio::error_code ec;
    auto endpoints = resolver.resolve(m_serverHost.hostName, m_serverHost.port, ec);
    if (ec)
    {
        LOG_ERROR("Resolve fail: {}", ec.message());
        return;
    }
    int attempts = 0;
    while (attempts < MAX_RETRIES && !m_session.Connect(endpoints))
    {
        ++attempts;
        LOG_INFO("Connection attempt {} failed. Retrying...", attempts);
        std::cout << "Connection attempt " << attempts << " failed. Retrying..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(5));
        if (attempts == MAX_RETRIES)
        {
            LOG_ERROR("Failed to connect after {} attempts to {}:{}. Max retries reached.",
                      MAX_RETRIES,
                      m_serverHost.hostName,
                      m_serverHost.port);
            return;
        }
    }
    m_isConnected = true;
    m_ioContext.run();
}

void ChatClient::OnResponse(Connection::Ptr session, std::size_t length)
{
    auto &buffer = session->GetReadBuf();
    auto protocolResponses = m_protocolCodec.UnPackProtocolResponse(buffer);
    for (const auto &protocolResponse : protocolResponses)
    {
        // HandleProtocolResponse
    }
}

void ChatClient::CloseConnection()
{
    if (m_isConnected)
    {
        m_session.CloseConnection();
    }
}
