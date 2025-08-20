#pragma once
#include <iostream>
#include "Logger.h"
#include <queue>
#include <asio.hpp>
#define CONNECTION_BUFFER_SIZE 1024

class Connection : public std::enable_shared_from_this<Connection>, public UncopybleAndUnmovable
{
public:
	using Ptr = std::shared_ptr<Connection>;
	using MessageCallback = std::function<void(const Connection::Ptr&, std::size_t)>;
	using DisConnctCallback = std::function<void(const Connection::Ptr&)>;

	Connection(asio::ip::tcp::socket&& socket);
	void AsyncReadMessage();
	void AsyncWriteMessage(const std::string& message);
	bool Connect(asio::ip::tcp::resolver::results_type endpoint);
	void SetMessageCallback(const MessageCallback&);
	void SetDisConnectCallback(const DisConnctCallback&); //被动断开
	void CloseConnection(); //主动断开
	uint32_t GetConnId() const;
	std::vector<char>& GetReadBuf();
	std::queue<std::string>& GetSendQueue();
	~Connection();

private:
	void AsyncWriteNextMessage();

	uint32_t m_connId;
	asio::ip::tcp::socket m_socket;
	std::vector<char> m_readBuf;
	std::queue<std::string> m_sendQueue;
	std::mutex m_sendQueMtx;
	std::condition_variable m_sendQueCV;

	MessageCallback m_onMessage;
	DisConnctCallback m_disConnect;
};