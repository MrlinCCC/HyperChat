#pragma once
#include "Session.h"

class ChatRoom
{
public:
    ChatRoom(size_t id, std::string name);
    ~ChatRoom();
    void Join(std::shared_ptr<Session> client);
    void Leave(int clientSocket);
    void BroadcastMessage(const std::string &message);

private:
    size_t m_Id;
    std::string m_name;
    std::unordered_map<size_t, std::shared_ptr<Session>> m_sessions;
};