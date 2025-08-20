#include "Connection.h"
#include "Utils.hpp"

Connection::Connection(asio::ip::tcp::socket&& socket) : m_socket(std::move(socket))
{
	m_readBuf.resize(CONNECTION_BUFFER_SIZE);
	m_connId = GenerateAutoIncrementId<Connection>();
}

Connection::~Connection()
{
	CloseConnection();
}

void Connection::AsyncReadMessage()
{
	auto self = shared_from_this();
	m_socket.async_read_some(asio::buffer(m_readBuf), [self](std::error_code ec, std::size_t length)
		{
			if (ec == asio::error::operation_aborted) {
				return;
			}
			if (ec == asio::error::eof || ec == asio::error::connection_reset) {
				if (self->m_disConnect) self->m_disConnect(self);
				return;
			}
			if (ec)
			{
				LOG_ERROR(ec.message());
				self->CloseConnection();
				return;
			}
			if (self->m_onMessage) {
				self->m_onMessage(self, length);
			}
			else
				LOG_WARN("OnMessage Callback is nullptr!");

			self->AsyncReadMessage(); });
}

void Connection::AsyncWriteMessage(const std::string& message)
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

void Connection::AsyncWriteNextMessage()
{
	std::lock_guard<std::mutex> lock(m_sendQueMtx);
	if (m_sendQueue.empty())
		return;

	const std::string& message = m_sendQueue.front();
	auto self = shared_from_this();
	asio::async_write(m_socket, asio::buffer(message.data(), message.size()), [self](std::error_code ec, std::size_t length)
		{
			if (ec == asio::error::operation_aborted) {
				return;
			}
			if (ec == asio::error::connection_reset) {
				if (self->m_disConnect) self->m_disConnect(self);
				return;
			}
			if (ec)
			{
				LOG_ERROR(ec.message());
				self->CloseConnection();
				return;
			}
			{
				std::lock_guard<std::mutex> lock(self->m_sendQueMtx);
				self->m_sendQueue.pop();
			}
			bool hasMoreMessages;
			{
				std::lock_guard<std::mutex> lock(self->m_sendQueMtx);
				hasMoreMessages = !self->m_sendQueue.empty();
			}
			if (hasMoreMessages)
				self->AsyncWriteNextMessage();
			else {
				self->m_sendQueCV.notify_one();
			} });
}

bool Connection::Connect(asio::ip::tcp::resolver::results_type endpoint)
{
	asio::error_code ec;
	asio::connect(m_socket, endpoint, ec);
	if (ec)
	{
		LOG_ERROR("Connect server fail: {}", ec.message());
		return false;
	}
	return true;
}

void Connection::CloseConnection()
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

void Connection::SetMessageCallback(const MessageCallback& onMessage)
{
	m_onMessage = onMessage;
}

void Connection::SetDisConnectCallback(const DisConnctCallback& disConnect) {
	m_disConnect = disConnect;
}

std::vector<char>& Connection::GetReadBuf()
{
	return m_readBuf;
}

std::queue<std::string>& Connection::GetSendQueue() {
	return m_sendQueue;
}

uint32_t Connection::GetConnId() const
{
	return m_connId;
}
