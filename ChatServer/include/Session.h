
#pragma once
#include <iostream>
#include <User.hpp>
#include "Logger.h"
#include <queue>
#include <asio.hpp>
#include <ProtocolMessage.h>

#define INIT_BODY_BUFFER_SIZE 1024

class Session : public std::enable_shared_from_this<Session>
{
public:
    using ptr = std::shared_ptr<Session>;
    using SubmitProtocolMessageCallback = std::function<
        void(std::shared_ptr<Session>, const ProtocolMessage &)>;

    Session(asio::ip::tcp::socket &&socket, SubmitProtocolMessageCallback callback);
    Session(Session &) = delete;
    Session &operator=(Session &) = delete;
    Session(Session &&) noexcept = default;
    Session &operator=(Session &&) noexcept = default;
    void AsyncReadMessage();
    void AsyncWriteMessage(ProtocolMessage &message);
    void CloseSession();
    uint32_t getSessionId();

    ~Session();

private:
    void AsyncReadMessageBody();
    void AsyncWriteNextMessage();

    uint32_t m_sessionId;
    asio::ip::tcp::socket m_socket;
    MessageHeader m_headerBuf;
    std::string m_bodyBuf;
    std::queue<std::string> m_sendQueue;
    std::mutex m_mtx;
    std::condition_variable m_cv;

    SubmitProtocolMessageCallback m_callback;
};