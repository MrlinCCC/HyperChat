#pragma once
#include "functional"
#include "Connection.h"
#include "ThreadPool.h"
#include <IoContextPool.h>
#include "UncopybleAndUnmovable.h"
#include "Protocol.h"
#include "Globals.hpp"

class AsyncTcpServer : public UncopybleAndUnmovable
{
public:
	AsyncTcpServer(unsigned short port, size_t rwThreadNum = RW_THREAD_NUM);
	~AsyncTcpServer();
	void Run();
	void Shutdown();
	void SetOnMessage(Connection::MessageCallback onMessage);
	void SetDisConnection(Connection::DisConnectCallback disConnect);
	size_t GetConnectionCount();

private:
	void AsyncAcceptConnection();

	void DisConnect(const Connection::Ptr &);

	// 单独线程处理连接，避免连接卡顿
	asio::io_context m_connContext;
	asio::executor_work_guard<asio::io_context::executor_type> m_workGuard;
	asio::ip::tcp::acceptor m_acceptor;
	// 多个线程进行异步读写，提高并发性能
	IoContextPool m_rwContextPool;
	std::unordered_map<uint32_t, Connection::Ptr> m_connections;
	std::mutex m_connsMtx;
	std::atomic<bool> m_isRunning;
	BinarySemaphore m_allAsyncFinish;

	Connection::MessageCallback m_onMessage;
	Connection::DisConnectCallback m_disConnect;
};