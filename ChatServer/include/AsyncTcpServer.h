#pragma once
#include "functional"
#include "Connection.h"
#include "ThreadPool.h"
#include <IoContextPool.h>
#include "UncopybleAndUnmovable.h"
#include "Protocol.h"
#include "Configuration.hpp"

class AsyncTcpServer : public UncopybleAndUnmovable
{
public:
	using ConnectCallback = std::function<void(const Connection::Ptr &)>;

	AsyncTcpServer(unsigned short port, size_t rwThreadNum = RW_THREAD_NUM);
	~AsyncTcpServer();
	void Run();
	void Shutdown();

	inline size_t GetConnectionCount()
	{
		std::lock_guard<std::mutex> lock(m_connsMtx);
		return m_connections.size();
	}

	inline void SetOnConnection(ConnectCallback onConnection)
	{
		m_onConnection = onConnection;
	}

	inline void SetOnMessage(Connection::MessageCallback onMessage)
	{
		m_onMessage = onMessage;
	}

	inline void SetOnCloseConnection(Connection::CloseCallback onCloseConnection)
	{

		m_onCloseConnection = onCloseConnection;
	}

	inline void SetOnError(Connection::ErrorCallback onError)
	{
		m_onError = onError;
	}

private:
	void AsyncAcceptConnection();

	void HandleConnection(const Connection::Ptr &conn);
	void HandleCloseConnection(const Connection::Ptr &);

	// 单独线程处理连接，避免连接卡顿
	asio::io_context m_connContext;
	asio::executor_work_guard<asio::io_context::executor_type> m_workGuard;
	asio::ip::tcp::acceptor m_acceptor;
	// 多个线程进行异步读写，每个线程一个ioContext，避免资源竞争
	IoContextPool m_rwContextPool;
	std::unordered_map<uint32_t, Connection::Ptr> m_connections;
	std::mutex m_connsMtx;
	std::atomic<bool> m_isRunning;

	ConnectCallback m_onConnection;
	Connection::MessageCallback m_onMessage;
	Connection::ErrorCallback m_onError;
	Connection::CloseCallback m_onCloseConnection;
};