#pragma once
#include <Protocol.h>
#include <functional>
#include <Session.h>
#include "UncopybleAndUnmovable.h"
#include <Serializer.hpp>
#include "SQLiteCpp/SQLiteCpp.h"

class ChatService : public UncopybleAndUnmovable
{
public:
    using ServiceHandler = std::function<ProtocolResponseMessage::Ptr(const std::shared_ptr<Session> &, const ProtocolRequestMessage::Ptr &)>;

    static ChatService &GetInstance();
    template <typename Handler, typename RequestType, typename ResponseType>
    void RegisterServiceHandler(MethodType type, Handler &&handler)
    {
        static_assert(std::is_invocable_r_v<ResponseType, Handler, RequestType>,
                      "Handler must be callable with RequestType and return ResponseType");
        if (m_serviceHandlerMap.find(type) != m_serviceHandlerMap.end())
        {
            LOG_WARN("Handler for this MethodType already exists!");
            return;
        }
        m_serviceHandlerMap.insert(type, [handler](const std::shared_ptr<Session> session &, const ProtocolMessage::Ptr &message)
                                   {
                                    RequestType req = Serializer::DeSerialize<RequestType>(message->m_payload);
                                    ResponseType response = handler(req);
                                    ProtocolResponseMessage responseMsg;
                                    responseMsg.m_header.m_status = Success;
                                    responseMsg.m_payload = Serializer::Serialize(response);
                                    return responseMsg; });
    }

    ProtocolResponseMessage::Ptr ExecuteService(const std::shared_ptr<Session> &session, const ProtocolRequestMessage::Ptr &ProtocolRequest);

private:
    ChatService();

    void InitDatabase();

    void CreateTables();

    ~ChatService() = default;

    std::unordered_map<MethodType, ServiceHandler> m_serviceHandlerMap;
    std::shared_ptr<SQLite::Database> p_db;
};