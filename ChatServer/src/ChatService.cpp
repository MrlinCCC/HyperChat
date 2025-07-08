#include "ChatService.h"

ChatService::ChatService()
{
    m_exeServiceCb = std::bind(&ChatService::HandleChatService, this, std::placeholders::_1);
}

ChatService &ChatService::GetInstance()
{
    static ChatService service;
    return service;
}

ProtocolMessage ChatService::ExecuteService(const ProtocolMessage &message) const
{
    return m_exeServiceCb(message);
}

void ChatService::SetExecuteServiceCb(ExecuteServiceCb callback)
{
    if (callback)
        m_exeServiceCb = callback;
}

ProtocolMessage ChatService::HandleChatService(const ProtocolMessage &message) const
{
    MessageType type = message.m_header.m_msgType;
    auto handlerIt = m_serviceHandlerMap.find(type);
    if (handlerIt != m_serviceHandlerMap.end())
    {
        return handlerIt->second(message);
    }
    ProtocolMessage response;
    response.m_header = message.m_header;
    response.m_header.m_status = Status::NOT_FOUND;
    response.p_body = MessageBodyFactory::Instance().Create(response.m_header.m_bodyType);
    return response;
}
