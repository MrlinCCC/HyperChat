#ifndef CHATCLIENT_H
#define CHATCLIENT_H
#include "iostream"
#include "Protocol.h"
#include "asio.hpp"
#include "Connection.h"
#define MAX_RETRIES 10

struct Host
{
    std::string hostName;
    std::string port;
    Host(const std::string &h, const std::string &p)
        : hostName(h), port(p)
    {
    }
};

class ChatClient
{
public:
    ChatClient(const Host &server);
    ~ChatClient();

    void ConnectServer();
    bool SendMessage(const std::string &message);
    void CloseConnection();

private:
    void OnResponse(Connection::Ptr session, std::size_t length);

    asio::io_context m_ioContext;
    asio::executor_work_guard<asio::io_context::executor_type> m_workGuard;
    Host m_serverHost;
    Connection m_session;
    bool m_isConnected;

    ProtocolCodec m_protocolCodec;
};

#endif // CHATCLIENT_H
