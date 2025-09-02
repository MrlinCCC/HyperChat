#include "Connection.h"
#include "Utils.hpp"

Connection::Connection(asio::ip::tcp::socket &&socket) : m_socket(std::move(socket))
{
	m_connId = GenerateAutoIncrementId<Connection>();
}

Connection::~Connection()
{
	if (m_state == ConnectionState::CONNECTED)
		CloseConnection();
}

void Connection::AsyncReadMessage()
{
	if (m_state != ConnectionState::CONNECTED) // 连接开始关闭后不再继续注册异步读
	{
		LOG_WARN("Attempting to read from a disconnected, closing or closed connection.");
		return;
	}
	auto self = shared_from_this();
	m_socket.async_read_some(asio::buffer(m_readBuf), [self](std::error_code ec, std::size_t length)
							 {
			if (ec == asio::error::operation_aborted) //主动关闭后清理已注册的异步读时触发
				return;
			if (ec == asio::error::eof || ec == asio::error::connection_reset) { //对方关闭时触发
				self->HandleOnCloseConnection();
				return;
			}
			if (ec)
			{
				LOG_ERROR(ec.message());
				if(self->m_onError) self->m_onError(self);
				self->CloseConnection();
				return;
			}
			if (self->m_onMessage) self->m_onMessage(self,length);

			self->AsyncReadMessage(); });
}

void Connection::AsyncWriteMessage(const std::string &message)
{
	if (m_state != ConnectionState::CONNECTED)
	{
		LOG_WARN("Attempting to read from a disconnected, closing or closed connection.");
		return;
	}
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

	const std::string &message = m_sendQueue.front();
	auto self = shared_from_this();
	asio::async_write(m_socket, asio::buffer(message.data(), message.size()), [self](std::error_code ec, std::size_t length)
					  {
			if (ec == asio::error::operation_aborted) {
				return;
			}
			if (ec == asio::error::connection_reset) {
				self->HandleOnCloseConnection();
				return;
			}
			if (ec)
			{
				LOG_ERROR(ec.message());
				if(self->m_onError) self->m_onError(self);
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
	m_state = ConnectionState::CONNECTED;
	return true;
}

bool Connection::Connect(const std::string &host, uint16_t port)
{
	asio::error_code ec;
	asio::ip::tcp::endpoint endpoint(asio::ip::make_address(host), port);
	m_socket.connect(endpoint, ec);
	if (ec)
	{
		LOG_ERROR("Connect server fail: {}", ec.message());
		return false;
	}
	m_state = ConnectionState::CONNECTED;
	return true;
}

void Connection::CloseConnection()
{

	ConnectionState expected = ConnectionState::CONNECTED;
	if (!m_state.compare_exchange_strong(expected, ConnectionState::CLOSING))
	{
		LOG_WARN("Connection is already closing or closed.");
		return;
	}
	{
		std::unique_lock<std::mutex> lock(m_sendQueMtx);
		if (!m_sendQueCV.wait_for(lock, std::chrono::seconds(DelayCloseTimeout),
								  [this]
								  { return m_sendQueue.empty(); }))
		{
			LOG_WARN("Send queue not empty, force closing.");
		}
	}
	if (m_socket.is_open())
	{
		m_socket.cancel();
		m_socket.close();
	}

	if (m_onClose)
		m_onClose(shared_from_this());

	m_state = ConnectionState::CLOSED;
}

void Connection::HandleOnCloseConnection()
{
	ConnectionState expected = ConnectionState::CONNECTED;
	if (!m_state.compare_exchange_strong(expected, ConnectionState::CLOSING))
	{
		LOG_WARN("Connection is already closing or closed.");
		return;
	}
	if (m_socket.is_open())
	{
		m_socket.cancel();
	}
	std::fill(std::begin(m_readBuf), std::end(m_readBuf), 0); // clear
	{
		std::lock_guard<std::mutex> lock(m_sendQueMtx);
		while (!m_sendQueue.empty())
		{
			m_sendQueue.pop(); // drop
		}
	}
	if (m_socket.is_open())
	{
		m_socket.close();
	}

	auto self = shared_from_this();
	if (m_onClose)
		m_onClose(self);

	m_state = ConnectionState::CLOSED;
}