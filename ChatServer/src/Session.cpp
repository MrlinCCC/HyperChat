#include "Session.h"
#include "Utils.hpp"

Session::Session(std::shared_ptr<Connection> conn) : m_conn(conn),
                                                     m_codec(std::make_unique<ProtocolCodec>()), m_lastReadTime(std::chrono::steady_clock::now())
{
    m_sessionId = GenerateAutoIncrementId<Session>();
}

void Session::SendFrame(const ProtocolFrame::Ptr &frame)
{
    if (m_state != SessionState::ACTIVE)
        return;

    auto msg = m_codec->PackProtocolFrame(frame);
    m_conn->AsyncWriteMessage(msg);
}

void Session::CloseSession()
{
    SessionState expected = SessionState::ACTIVE;
    if (!m_state.compare_exchange_strong(expected, SessionState::CLOSING))
    {
        return;
    }
    m_conn->CloseConnection();
    // 主动关闭后仍然会触发connection的回调最终调用HandleOnCloseSession
}

void Session::HandleMessage(const Connection::Ptr &conn, std::size_t length)
{
    if (m_state != SessionState::ACTIVE)
        return;
    const auto &buffer = conn->GetReadBuf();
    auto frames = m_codec->UnPackProtocolFrame(buffer, length);
    for (const auto &frame : frames)
    {
        m_onProtocolFrame(shared_from_this(), frame);
    }
}

void Session::HandleOnCloseSession(const Connection::Ptr &conn)
{
    SessionState expected = SessionState::CLOSING;
    if (!m_state.compare_exchange_strong(expected, SessionState::CLOSED))
    {
        return;
    }
    // todo service response
    if (m_onCloseSession)
        m_onCloseSession(shared_from_this());
}