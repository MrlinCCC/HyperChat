#include "AsyncTcpServer.h"

AsyncTcpServer::AsyncTcpServer(unsigned short port, size_t rwThreadNum)
	: m_connContext(),
	  m_workGuard(asio::make_work_guard(m_connContext)),
	  m_acceptor(m_connContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)),
	  m_rwContextPool(rwThreadNum),
	  m_isRunning(false)
{
	m_connections.reserve(CONNECTION_SIZE);
}

AsyncTcpServer::~AsyncTcpServer()
{
	if (m_isRunning)
		Shutdown();
}

void AsyncTcpServer::Run()
{
	AsyncAcceptConnection();
	auto endpoint = m_acceptor.local_endpoint();
	LOG_INFO("TcpServer is running! Listening on {}:{}!", endpoint.address().to_string(), endpoint.port());
	m_isRunning = true;
	m_rwContextPool.Run();
	m_connContext.run();
}

void AsyncTcpServer::Shutdown()
{
	if (m_isRunning)
	{
		m_isRunning = false;
		if (m_acceptor.is_open())
		{
			m_acceptor.cancel();
			m_acceptor.close();
		}
		std::vector<Connection::Ptr> conns;
		{
			std::lock_guard<std::mutex> lock(m_connsMtx);
			for (auto &[id, conn] : m_connections)
				conns.push_back(conn);
		}
		for (auto &conn : conns)
			conn->CloseConnection();
		m_rwContextPool.Shutdown();
		m_workGuard.reset();
		LOG_INFO("TcpServer shutdown!");
	}
}

void AsyncTcpServer::AsyncAcceptConnection()
{
	auto socket = std::make_shared<asio::ip::tcp::socket>(m_connContext);
	m_acceptor.async_accept(*socket, [this, socket](const asio::error_code &ec) mutable
							{
			if (ec == asio::error::operation_aborted) {
				return;
			}
			if (ec) {
				LOG_ERROR(ec.message());
				return;
			}
			auto& ioContext = m_rwContextPool.GetIoContext();
			asio::ip::tcp::socket newSocket(ioContext);
			try {
				auto protocol = socket->local_endpoint().protocol();
				newSocket.assign(protocol, socket->release());
			}
			catch (const std::system_error& e) {
				LOG_ERROR("Failed to assign socket: " + std::string(e.what()));
				return;
			}

			auto newConnPtr = std::make_shared<Connection>(std::move(newSocket));
			newConnPtr->SetState(ConnectionState::CONNECTED);
			HandleConnection(newConnPtr);
			if (m_onMessage) newConnPtr->SetMessageCallback(m_onMessage);
			newConnPtr->SetCloseCallback(std::bind(&AsyncTcpServer::HandleCloseConnection, this, newConnPtr));
			if (m_onError) newConnPtr->SetErrorCallback(m_onError);
			newConnPtr->SetState(ConnectionState::CONNECTED);
			newConnPtr->AsyncReadMessage();
			auto remote = newConnPtr->GetSocket().remote_endpoint();
			LOG_INFO("Accepted new connection: remote={}:{}",remote.address().to_string().c_str(),remote.port());
			AsyncAcceptConnection(); });
}

void AsyncTcpServer::HandleConnection(const Connection::Ptr &conn)
{
	{
		std::lock_guard<std::mutex> lock(m_connsMtx);
		m_connections.insert(std::make_pair(conn->GetConnId(), conn));
	}
	if (m_onConnection)
		m_onConnection(conn);
}

void AsyncTcpServer::HandleCloseConnection(const Connection::Ptr &conn)
{
	{
		std::lock_guard<std::mutex> lock(m_connsMtx);
		if (m_connections.find(conn->GetConnId()) != m_connections.end())
		{
			m_connections.erase(conn->GetConnId());
		}
	}
	if (m_onCloseConnection)
		m_onCloseConnection(conn);
}