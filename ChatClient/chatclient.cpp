#include "chatclient.h"

ChatClient::ChatClient(const Host &server)
    : m_conn(std::make_shared<Connection>(asio::ip::tcp::socket(m_ioContext))),
      m_workGuard(asio::make_work_guard(m_ioContext)), m_serverHost(server), m_isConnected(false)
{
    auto onResponse = std::bind(&ChatClient::OnResponse, this, std::placeholders::_1, std::placeholders::_2);
    m_conn->SetMessageCallback(onResponse);
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
    auto endpoints = resolver.resolve(m_serverHost.hostName, std::to_string(m_serverHost.port), ec);
    if (ec)
    {
        LOG_ERROR("Resolve fail: {}", ec.message());
        return;
    }
    int attempts = 0;
    while (attempts < MAX_RETRIES && !m_conn->Connect(endpoints))
    {
        ++attempts;
        LOG_INFO("Connection attempt {} failed. Retrying...", attempts);
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

    emit connectedToServer();
}

void ChatClient::OnResponse(Connection::Ptr conn, std::size_t length)
{
    auto &buffer = m_conn->GetReadBuf();
    auto protocolResponses = ProtocolCodec::Instance().UnPackProtocolResponse(buffer, length);
    for (const auto &protocolResponse : protocolResponses)
    {
        if (!protocolResponse->m_requestId && !protocolResponse->m_pushType.empty())
        {
            // push type
            LOG_INFO("Push method success!");
        }
        else
        {
            // response type
            LOG_INFO("Response method success!");
        }
    }
}

void ChatClient::Run()
{
    m_conn->AsyncReadMessage();
    m_ioContext.run();
}

void ChatClient::CloseConnection()
{
    if (m_isConnected)
    {
        m_conn->CloseConnection();
    }
}

void ChatClient::Register(const std::string username, const std::string passwd)
{
    AuthRequest req{username, passwd};
    SendRequest<AuthRequest>(req, MethodType::Register);
}

void ChatClient::Login(const std::string username, const std::string passwd)
{
    AuthRequest req{username, passwd};
    SendRequest<AuthRequest>(req, MethodType::Login);
}
