#include "Session.h"
#include "IdGenerator.hpp"

Session::Session(asio::ip::tcp::socket &&socket, SubmitProtocolMessageCallback callback) : m_socket(std::move(socket)), m_callback(callback)
{
    m_sessionId = generateAutoIncrementId<Session>();
}

void Session::AsyncReadMessage()
{
    auto self = shared_from_this();
    asio::async_read(m_socket, asio::buffer(&m_headerBuf, sizeof(MessageHeader)), [self](std::error_code ec, std::size_t /*len*/)
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
                         self->AsyncReadMessageBody(); });
}

void Session::AsyncReadMessageBody()
{
    auto self = shared_from_this();
    m_bodyBuf.resize(m_headerBuf.m_bodyLength);
    asio::async_read(m_socket, asio::buffer(m_bodyBuf),
                     [self](const asio::error_code &ec, std::size_t /*len*/)
                     {
                         if (ec)
                         {
                             LOG_ERROR(ec.message());
                             self->CloseSession();
                             return;
                         }
                         try
                         {
                             ProtocolMessage message = ProtocolMessage::DeSerialize(self->m_headerBuf, self->m_bodyBuf);
                             if (!self->m_callback)
                             {
                                 LOG_WARN("Service callback is nullptr!");
                             }
                             self->m_callback(self, message);
                         }
                         catch (...)
                         {
                             LOG_ERROR("parse Message error");
                         }
                         self->AsyncReadMessage();
                     });
}

void Session::AsyncWriteMessage(ProtocolMessage &message)
{
    bool write_in_progress;
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        write_in_progress = !m_sendQueue.empty();
        m_sendQueue.push(message.Serialize());
    }
    if (!write_in_progress)
        AsyncWriteNextMessage();
}

void Session::AsyncWriteNextMessage()
{
    std::lock_guard<std::mutex> lock(m_mtx);
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
        std::lock_guard<std::mutex> lock(self->m_mtx);
        self->m_sendQueue.pop();
        if (!self->m_sendQueue.empty()){
            self->AsyncWriteNextMessage();
        }
        else{
            self->m_cv.notify_one();
        } });
}

void Session::CloseSession()
{
    std::unique_lock<std::mutex> lock(m_mtx);
    m_cv.wait(lock, [this]()
              { return m_sendQueue.empty(); });
    if (m_socket.is_open())
    {
        m_socket.cancel();
        m_socket.close();
    }
}

uint32_t Session::getSessionId()
{
    return m_sessionId;
}

Session::~Session()
{
    CloseSession();
}