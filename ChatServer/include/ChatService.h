#pragma once
#include <Protocol.h>
#include <functional>
#include <Session.h>
#include "UncopybleAndUnmovable.h"
#include <Serializer.hpp>

class ChatService : public UncopybleAndUnmovable
{
public:
    using ServiceHandler = std::function<ProtocolMessage::Ptr(const std::shared_ptr<Session> &, const ProtocolMessage::Ptr &)>;

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
        m_serviceHandlerMap.insert(type, [handler]()
                                   {
                                    //todo Serialize:ProtocolMessage->RequestType
                                    ResponseType response = handler(req);
                                    //todo Serialize:ResponseType->ProtocolMessage
                                    ProtocolMessage responseMsg;
                                    return responseMsg; });
    }

    ProtocolMessage::Ptr ExecuteService(const std::shared_ptr<Session> &session, const ProtocolMessage::Ptr &ProtocolMsg);

private:
    ChatService();
    ~ChatService() = default;

    std::unordered_map<MethodType, ServiceHandler> m_serviceHandlerMap;
};