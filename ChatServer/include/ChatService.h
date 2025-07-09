#pragma once
#include <ProtocolMessage.h>
#include <functional>
#include <Session.h>

class ChatService
{
public:
    using ExecuteServiceCb = std::function<ProtocolMessage(Session::ptr session, const ProtocolMessage &request)>;

    static ChatService &GetInstance();
    ProtocolMessage ExecuteService(Session::ptr session, const ProtocolMessage &request) const;
    template <typename F>
    void SetExecuteServiceCb(F &&callback)
    {
        m_exeServiceCb = std::forward<F>(callback);
    }

    template <typename F>
    void AddServiceHandler(MessageType type, F &&handler)
    {
        m_serviceHandlerMap[type] = std::forward<F>(handler);
    }

private:
    ChatService();
    ChatService(const ChatService &) = delete;
    ChatService &operator=(const ChatService &) = delete;
    ChatService(const ChatService &&) = delete;
    ChatService &operator=(const ChatService &&) = delete;
    ProtocolMessage HandleChatService(Session::ptr session, const ProtocolMessage &request) const;

    ExecuteServiceCb m_exeServiceCb;
    std::unordered_map<MessageType, ExecuteServiceCb> m_serviceHandlerMap;
};