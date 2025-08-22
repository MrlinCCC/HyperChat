#include "ChatServer.h"
#include "ChatService.h"

ChatServer::ChatServer(unsigned short port, size_t threadNum)
	: m_tcpServer(port),
	  m_servicePool(threadNum),
	  m_isRunning(false)
{
	auto handleProtocolMessage = std::bind(&ChatServer::OnProtocolRequest, this, std::placeholders::_1, std::placeholders::_2);
	m_tcpServer.SetOnMessage(handleProtocolMessage);
	m_tcpServer.SetDisConnection([](Connection::Ptr conn)
								 {
									 Status status;
									 ChatService::GetInstance().LogoutHandler(conn, UserIdRequest{conn->GetUserId()}, status);
									 if (status != SUCCESS)
									 {
										 LOG_ERROR("DisConnect logout fail error!");
									 }
									 conn->CloseConnection();
								 });
	m_handleProtocolRequest = std::bind(&ChatServer::HandleProtocolRequest, this, std::placeholders::_1, std::placeholders::_2);
}

ChatServer::~ChatServer()
{
	if (m_isRunning)
		Shutdown();
}

void ChatServer::Run()
{
	m_isRunning = true;
	m_tcpServer.Run();
}

void ChatServer::Shutdown()
{
	if (m_isRunning)
	{
		m_isRunning = false;
		m_tcpServer.Shutdown();
		m_servicePool.Shutdown();
	}
}

void ChatServer::SetHandleProtocolRequest(HandleProtocolRequestCallback handleProtocolRequest)
{
	if (handleProtocolRequest)
	{
		m_handleProtocolRequest = handleProtocolRequest;
	}
}

void ChatServer::OnProtocolRequest(const Connection::Ptr &connection, std::size_t length)
{
	auto &buffer = connection->GetReadBuf();
	auto protocolRequests = ProtocolCodec::Instance().UnPackProtocolRequest(buffer);
	for (const auto &protocolRequest : protocolRequests)
	{
		m_handleProtocolRequest(connection, protocolRequest);
	}
}

void ChatServer::HandleProtocolRequest(const Connection::Ptr &connection, const ProtocolRequest::Ptr &req)
{
	auto conn = connection->shared_from_this();
	if (m_isRunning)
	{
		m_servicePool.SubmitTask(
			[this](std::shared_ptr<Connection> connection, ProtocolRequest::Ptr req)
			{
				ProtocolResponse::Ptr protocolResponse = ChatService::GetInstance().DispatchService(connection, req);
				connection->AsyncWriteMessage(ProtocolCodec::Instance().PackProtocolResponse(protocolResponse));
			},
			conn, req);
	}
}