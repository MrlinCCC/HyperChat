#pragma once
#include <Protocol.h>
#include <functional>
#include <connection.h>
#include "UncopybleAndUnmovable.h"
#include <Serializer.hpp>
#include "SQLiteCpp/SQLiteCpp.h"

class ChatService : public UncopybleAndUnmovable
{
public:
    using ServiceHandler = std::function<ProtocolResponse::Ptr(const std::shared_ptr<Connection> &, const ProtocolRequest::Ptr &)>;

    static ChatService &GetInstance();
    template <typename Handler, typename RequestType, typename ResponseType>
    void RegisterServiceHandler(std::string method, Handler &&handler)
    {
        static_assert(std::is_invocable_r_v<ResponseType, Handler, RequestType>,
                      "Handler must be callable with RequestType and return ResponseType");
        if (m_serviceHandlerMap.find(method) != m_serviceHandlerMap.end())
        {
            LOG_WARN("Handler for this MethodType already exists!");
            return;
        }
        m_serviceHandlerMap.insert(method, [handler](const std::shared_ptr<Connection> connection &, const ProtocolMessage::Ptr &message)
                                   {
                                    RequestType req = Serializer::DeSerialize<RequestType>(message->m_payload);
                                    ResponseType response = handler(req);
                                    ProtocolResponse responseMsg;
                                    responseMsg.m_header.m_status = Success;
                                    responseMsg.m_payload = Serializer::Serialize(response);
                                    return responseMsg; });
    }

    ProtocolResponse::Ptr ExecuteService(const std::shared_ptr<Connection> &connection, const ProtocolRequest::Ptr &ProtocolRequest);

private:
    ChatService();

    void InitDatabase();

    void CreateTables();

    ~ChatService() = default;

    std::unordered_map<std::string, ServiceHandler> m_serviceHandlerMap;
    std::shared_ptr<SQLite::Database> p_db;
};