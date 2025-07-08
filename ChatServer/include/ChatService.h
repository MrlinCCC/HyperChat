#pragma once
#include <ProtocolMessage.h>
#include <functional>

class ChatService
{
    using ExecuteServiceCb = std::function<ProtocolMessage(const ProtocolMessage &message)>;

public:
    static ChatService &GetInstance();
    ProtocolMessage ExecuteService(const ProtocolMessage &message) const;
    void SetExecuteServiceCb(ExecuteServiceCb callback);

private:
    ChatService();
    ChatService(const ChatService &) = delete;
    ChatService &operator=(const ChatService &) = delete;
    ChatService(const ChatService &&) = delete;
    ChatService &operator=(const ChatService &&) = delete;
    ProtocolMessage HandleChatService(const ProtocolMessage &message) const;

    ExecuteServiceCb m_exeServiceCb;
    std::unordered_map<MessageType, ExecuteServiceCb> m_serviceHandlerMap;
};