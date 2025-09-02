#pragma once
#include <iostream>
#include "Connection.h"

enum class SessionState
{
    INIT,
    ACTIVE,
    CLOSING,
    CLOSED
};

enum class AuthState
{
    ANONYMOUS,
    AUTHENTICATED
};

class Session : public std::enable_shared_from_this<Session>
{
public:
    using Ptr = std::shared_ptr<Session>;
    using OnProtocolFrame = std::function<void(const Session::Ptr &, const ProtocolFrame::Ptr &)>;
    using OnCloseSession = std::function<void(const std::shared_ptr<Session> &)>;

    Session(std::shared_ptr<Connection> conn);

    void SendFrame(const ProtocolFrame::Ptr &frame);

    void CloseSession();

    void HandleMessage(const Connection::Ptr &, std::size_t);

    void HandleOnCloseSession(const Connection::Ptr &);

    inline void Session::SetProtocolHandler(OnProtocolFrame onProtocolFrame)
    {
        m_onProtocolFrame = onProtocolFrame;
    }

    inline void Session::SetOnCloseSession(OnCloseSession onCloseSession)
    {
        m_onCloseSession = onCloseSession;
    }

    inline uint32_t GetSessionId() const
    {
        return m_sessionId;
    }

    inline void SetUserId(uint32_t id)
    { // 绑定登录用户Id
        m_userId = id;
    }

    inline uint32_t GetUserId() const
    {
        return m_userId;
    }

    inline SessionState GetState() const
    {
        return m_state;
    }

    inline void SetState(SessionState state)
    {
        m_state = state;
    }

    inline AuthState GetAuthState() const
    {
        return m_authState;
    }

    inline void SetAuthState(AuthState state)
    {
        m_authState = state;
    }

private:
    uint32_t m_userId;
    uint32_t m_sessionId;

    std::shared_ptr<Connection> m_conn;
    std::unique_ptr<ProtocolCodec> m_codec;

    OnProtocolFrame m_onProtocolFrame;
    OnCloseSession m_onCloseSession;

    std::atomic<SessionState> m_state{SessionState::INIT};
    std::atomic<AuthState> m_authState{AuthState::ANONYMOUS};
};