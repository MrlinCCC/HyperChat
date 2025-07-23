#include "ChatService.h"

ChatService::ChatService()
{
}

ChatService &ChatService::GetInstance()
{
    static ChatService service;
    return service;
}

ProtocolMessage::Ptr ChatService::ExecuteService(const std::shared_ptr<Session> &session, const ProtocolMessage::Ptr &ProtocolMsg)
{
    MethodType type = ProtocolMsg->m_header.m_methodType;
    if (m_serviceHandlerMap.find(type) != m_serviceHandlerMap.end())
    {
        LOG_WARN("Handler for this MethodType not found!");
        // todo router to method no found service
        return nullptr;
    }
    return m_serviceHandlerMap[type](session, ProtocolMsg);
}