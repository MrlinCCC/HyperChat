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

ProtocolMessage ChatService::ExecuteService(Session::ptr session, const ProtocolMessage &request) const
{
    return m_exeServiceCb(session, request);
}

ProtocolMessage ChatService::HandleChatService(Session::ptr session, const ProtocolMessage &request) const
{
    MessageType type = request.m_header.m_msgType;
    auto handlerIt = m_serviceHandlerMap.find(type);
    if (handlerIt != m_serviceHandlerMap.end())
    {
        return handlerIt->second(session, request);
    }
    ProtocolMessage response;
    response.m_header = request.m_header;
    response.m_header.m_status = Status::NOT_FOUND;
    response.p_body = MessageBodyFactory::Instance().Create(response.m_header.m_bodyType);
    return response;
}
