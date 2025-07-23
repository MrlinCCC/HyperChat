#include "Session.h"
#include "utils/IdGenerator.hpp"

Session::Session(asio::ip::tcp::socket &&socket) : m_socket(std::move(socket))
{
    m_readBuf.resize(BUFFER_SIZE);
    m_sessionId = generateAutoIncrementId<Session>();
}

Session::~Session()
{
    CloseSession();
}

void Session::AsyncReadMessage()
{
    auto self = shared_from_this();
    m_socket.async_read_some(asio::buffer(m_readBuf), [self](std::error_code ec, std::size_t length)
                             {
                        if (ec == asio::error::eof || ec == asio::error::operation_aborted) {
                            return;
                        }
                         if (ec)
                         {
                             LOG_ERROR(ec.message());
                             self->CloseSession();
                             return;
                         }
                         if(!self->m_onMessage){
                            LOG_WARN("OnMessage Callback is nullptr!");
                         }
                         self->m_onMessage(self,length);
                         self->AsyncReadMessage(); });
}

void Session::AsyncWriteMessage(const std::string &message)
{
    bool write_in_progress;
    {
        std::lock_guard<std::mutex> lock(m_sendQueMtx);
        write_in_progress = !m_sendQueue.empty();
        m_sendQueue.push(message);
    }
    if (!write_in_progress)
        AsyncWriteNextMessage();
}

void Session::AsyncWriteNextMessage()
{
    std::lock_guard<std::mutex> lock(m_sendQueMtx);
    if (m_sendQueue.empty())
        return;

    const std::string &message = m_sendQueue.front();
    auto self = shared_from_this();
    asio::async_write(m_socket, asio::buffer(message.data(), message.size()), [self](std::error_code ec, std::size_t length)
                      {
         if (ec)
        {
            LOG_ERROR(ec.message());
            self->CloseSession();
            return;
        }
        std::lock_guard<std::mutex> lock(self->m_sendQueMtx);
        self->m_sendQueue.pop();
        if (!self->m_sendQueue.empty()){
            self->AsyncWriteNextMessage();
        }
        else{
            self->m_sendQueCV.notify_one();
        } });
}

void Session::CloseSession()
{
    std::unique_lock<std::mutex> lock(m_sendQueMtx);
    m_sendQueCV.wait(lock, [this]()
                     { return m_sendQueue.empty(); });
    if (m_socket.is_open())
    {
        m_socket.cancel();
        m_socket.close();
    }
}

void Session::SetMessageCallback(const MessageCallback &onMessage)
{
    m_onMessage = onMessage;
}

uint32_t Session::GetSessionId()
{
    return m_sessionId;
}

std::string &Session::GetReadBuf()
{
    return m_readBuf;
}