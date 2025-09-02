#pragma once
#include <iostream>
#include "Logger.h"
#include <queue>
#include <asio.hpp>
#include "Protocol.h"

constexpr const size_t ConnectionBufferSize = 1024;
constexpr const size_t DelayCloseTimeout = 1;

enum class ConnectionState
{
	DISCONNECTED,
	CONNECTED,
	CLOSING,
	CLOSED
};

class Connection : public std::enable_shared_from_this<Connection>
{
public:
	using Ptr = std::shared_ptr<Connection>;
	using MessageCallback = std::function<void(const Connection::Ptr &, std::size_t)>;
	using CloseCallback = std::function<void(const Connection::Ptr &)>;
	using ErrorCallback = std::function<void(const Connection::Ptr &)>;

	Connection(asio::ip::tcp::socket &&socket);
	void AsyncReadMessage();
	void AsyncWriteMessage(const std::string &message);
	// 阻塞连接
	bool Connect(asio::ip::tcp::resolver::results_type endpoint);
	bool Connect(const std::string &host, uint16_t port);

	void CloseConnection(); // 主动断开
	inline ConnectionState GetState() const
	{
		return m_state;
	}
	inline void SetState(ConnectionState state)
	{
		m_state = state;
	}
	inline void SetMessageCallback(const MessageCallback &onMessage)
	{
		m_onMessage = onMessage;
	}
	inline void SetCloseCallback(const CloseCallback &onClose)
	{
		m_onClose = onClose;
	}
	inline void SetErrorCallback(const ErrorCallback &onError)
	{
		m_onError = onError;
	}
	inline const asio::ip::tcp::socket &GetSocket() const
	{
		return m_socket;
	}
	inline uint32_t GetConnId() const
	{
		return m_connId;
	}
	inline const char *GetReadBuf() const
	{ // 用于协议解析
		return m_readBuf;
	}
	inline std::queue<std::string> &GetSendQueue()
	{ // 用于测试
		return m_sendQueue;
	}
	~Connection();

private:
	void AsyncWriteNextMessage();
	void HandleOnCloseConnection(); // 被动断开回调

	uint32_t m_connId;
	asio::ip::tcp::socket m_socket;
	char m_readBuf[ConnectionBufferSize];
	std::queue<std::string> m_sendQueue;
	std::mutex m_sendQueMtx;
	std::condition_variable m_sendQueCV;

	MessageCallback m_onMessage;
	CloseCallback m_onClose;
	ErrorCallback m_onError;

	std::atomic<ConnectionState> m_state{ConnectionState::DISCONNECTED};
};