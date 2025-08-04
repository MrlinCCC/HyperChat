
#pragma once
#include <iostream>
#include "Logger.h"
#include <queue>
#include <asio.hpp>

#define BUFFER_SIZE 1024

class Connection : public std::enable_shared_from_this<Connection>, public UncopybleAndUnmovable
{
public:
    using Ptr = std::shared_ptr<Connection>;
    using MessageCallback = std::function<void(Connection::Ptr, std::size_t)>;

    Connection(asio::ip::tcp::socket &&socket);
    void AsyncReadMessage();
    void AsyncWriteMessage(const std::string &message);
    bool Connect(asio::ip::tcp::resolver::results_type endpoint);
    void SetMessageCallback(const MessageCallback &);
    void CloseConnection();
    uint32_t GetConnId() const;
    std::string &GetReadBuf();
    ~Connection();

private:
    void AsyncWriteNextMessage();

    uint32_t m_connId;
    asio::ip::tcp::socket m_socket;
    std::string m_readBuf;
    std::queue<std::string> m_sendQueue;
    std::mutex m_sendQueMtx;
    std::condition_variable m_sendQueCV;

    MessageCallback m_onMessage;
};