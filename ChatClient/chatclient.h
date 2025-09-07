#ifndef CHATCLIENT_H
#define CHATCLIENT_H

#include "iostream"
#include "Protocol.h"
#include "asio.hpp"
#include "Connection.h"
#include <QObject>
#include "Dto.hpp"
#include "Serializer.hpp"
#include "unordered_map"

constexpr int MaxRetries = 10;

struct Host
{
    std::string hostName;
    unsigned short port;
    Host(const std::string &h, unsigned short p)
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
    void RegisterFinishAction();
    void Login(const std::string username, const std::string passwd);
    void LoginFinishAction();

private:
    void OnResponse(Connection::Ptr conn, std::size_t length);

    template <typename BodyT>
    void SendRequest(BodyT body, MethodType method)
    {
        auto protocolReq = std::make_shared<ProtocolFrame>();
        protocolReq->m_header.m_type = FrameType::REQUEST;
        protocolReq->m_header.m_method = method;
        protocolReq->m_payload = Serializer::Serialize<BodyT>(body);
        m_conn.AsyncWriteMessage(m_codec.PackProtocolFrame(protocolReq));
    }

    asio::io_context m_ioContext;
    asio::executor_work_guard<asio::io_context::executor_type> m_workGuard;
    Host m_serverHost;
    Connection m_conn;
    ProtocolCodec m_codec;
    bool m_isConnected;

    std::unordered_map<MethodType, std::function<void()>> m_pushActions;
    std::unordered_map<MethodType, std::function<void()>> m_responseActions;
};

#endif // CHATCLIENT_H
