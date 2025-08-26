#pragma once
#include <iostream>
#include "Logger.h"
#include <queue>
#include <asio.hpp>
#define CONNECTION_BUFFER_SIZE 1024

enum class ConnectionState
{
	DISCONNECTED,
	CONNECTED,
	CLOSING,
	CLOSED
};

class Connection : public std::enable_shared_from_this<Connection>, public UncopybleAndUnmovable
{
public:
	using Ptr = std::shared_ptr<Connection>;
	using MessageCallback = std::function<void(const Connection::Ptr &, std::size_t)>;
	using DisConnectCallback = std::function<void(const Connection::Ptr &)>;

	Connection(asio::ip::tcp::socket &&socket);
	void AsyncReadMessage();
	void AsyncWriteMessage(const std::string &message);
	bool Connect(asio::ip::tcp::resolver::results_type endpoint);
	bool Connect(const std::string &host, uint16_t port);
	void CloseConnection(); // 主动断开
	inline ConnectionState GetState(ConnectionState state)
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
	inline void SetDisConnectCallback(const DisConnectCallback &disConnect) // 被动断开
	{
		m_disConnect = disConnect;
	}
	inline const asio::ip::tcp::socket &GetSocket() const
	{
		return m_socket;
	}
	inline uint32_t GetConnId() const
	{
		return m_connId;
	}
	inline void SetUserId(uint32_t id)
	{ // 绑定登录用户Id
		m_userId = id;
	}
	inline uint32_t GetUserId() const
	{
		return m_userId;
	}
	inline std::vector<char> &GetReadBuf()
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

	uint32_t m_connId;
	uint32_t m_userId;
	asio::ip::tcp::socket m_socket;
	std::vector<char> m_readBuf;
	std::queue<std::string> m_sendQueue;
	std::mutex m_sendQueMtx;
	std::condition_variable m_sendQueCV;

	MessageCallback m_onMessage;
	DisConnectCallback m_disConnect;

	ConnectionState m_state;
};