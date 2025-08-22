#ifndef CHATCLIENT_H
#define CHATCLIENT_H
#include "iostream"
#include "Protocol.h"
#include "asio.hpp"
#include "Connection.h"
#define MAX_RETRIES 10
#include <QObject>
#include "Dto.hpp"
#include "Serializer.hpp"
#include "unordered_map"

struct Host
{
    std::string hostName;
    std::string port;
    Host(const std::string &h, const std::string &p)
        : hostName(h), port(p)
    {
    }
};

class ChatClient : public QObject
{
    Q_OBJECT
signals:
    void connectedToServer();

public:
    ChatClient(const Host &server);
    ~ChatClient();

    void ConnectServer();
    void Run();
    void CloseConnection();

    void Register(const std::string username, const std::string passwd);
    void OnRegisterAction();
    void Login(const std::string username, const std::string passwd);
    void OnLoginAction();

private:
    void OnResponse(Connection::Ptr conn, std::size_t length);

    template <typename BodyT>
    void SendRequest(BodyT body, const std::string &method)
    {
        auto protocolReq = std::make_shared<ProtocolRequest>();
        protocolReq->m_method = method;
        protocolReq->m_payload = Serializer::Serialize<BodyT>(body);
        m_conn->AsyncWriteMessage(ProtocolCodec::Instance().PackProtocolRequest(protocolReq));
    }

    asio::io_context m_ioContext;
    asio::executor_work_guard<asio::io_context::executor_type> m_workGuard;
    Host m_serverHost;
    Connection::Ptr m_conn;
    bool m_isConnected;

    std::unordered_map<std::string, std::function<void()>> m_pushActions;
    std::unordered_map<std::string, std::function<void()>> m_responseActions;
};

#endif // CHATCLIENT_H
