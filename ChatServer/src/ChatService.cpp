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
    return message;
}
