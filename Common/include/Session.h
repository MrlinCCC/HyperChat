
#pragma once
#include <iostream>
#include "Logger.h"
#include <queue>
#include <asio.hpp>

#define BUFFER_SIZE 1024

class Session : public std::enable_shared_from_this<Session>, public UncopybleAndUnmovable
{
public:
    using Ptr = std::shared_ptr<Session>;
    using MessageCallback = std::function<void(Session::Ptr, std::size_t)>;

    Session(asio::ip::tcp::socket &&socket);
    void AsyncReadMessage();
    void AsyncWriteMessage(const std::string &message);
    bool Connect(asio::ip::tcp::resolver::results_type endpoint);
    void SetMessageCallback(const MessageCallback &);
    void CloseSession();
    uint32_t GetSessionId();
    std::string &GetReadBuf();

    ~Session();

private:
    void AsyncWriteNextMessage();

    uint32_t m_sessionId;
    asio::ip::tcp::socket m_socket;
    std::string m_readBuf;
    std::queue<std::string> m_sendQueue;
    std::mutex m_sendQueMtx;
    std::condition_variable m_sendQueCV;

    MessageCallback m_onMessage;
};