#include "ChatServer.h"
#include "ChatService.h"
#include <chrono>

ChatServer::ChatServer(unsigned short port, size_t threadNum)
	: m_tcpServer(port),
	  m_servicePool(threadNum),
	  m_isRunning(false)
{
	auto onConnection = std::bind(&ChatServer::HandleConnection, this, std::placeholders::_1);
	m_tcpServer.SetOnConnection(onConnection);
	auto onCloseConnection = std::bind(&ChatServer::HandleCloseConnection, this, std::placeholders::_1);
	m_tcpServer.SetOnCloseConnection(onCloseConnection);
	m_handleProtocolFrame = std::bind(&ChatServer::HandleProtocolFrame, this, std::placeholders::_1, std::placeholders::_2);
}

ChatServer::~ChatServer()
{
	if (m_isRunning)
		Shutdown();
}

void ChatServer::Run()
{
	m_isRunning = true;
	m_timer.Start();
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

void ChatServer::HandleConnection(const Connection::Ptr &conn)
{
	auto newSession = std::make_shared<Session>(conn);
	auto onMessage = std::bind(&Session::HandleMessage, newSession, std::placeholders::_1, std::placeholders::_2);
	conn->SetMessageCallback(onMessage);
	newSession->SetProtocolHandler(m_handleProtocolFrame);
	auto onCloseSession = std::bind(&ChatServer::HandleCloseSession, this, std::placeholders::_1);
	newSession->SetOnCloseSession(onCloseSession);
	newSession->SetState(SessionState::ACTIVE);
	{
		std::lock_guard<std::mutex> lock(m_sessionMtx);
		m_sessionMap.insert(std::make_pair(newSession->GetSessionId(), newSession));
	}
	{
		std::lock_guard<std::mutex> lock(m_connIdSessionMtx);
		m_connIdToSession.insert(std::make_pair(conn->GetConnId(), newSession));
	}
	m_timer.RunEvery(SESSION_ALIVE_CHECK_INTERNAL_MS, [newSession]()
					 { 
						auto now = std::chrono::steady_clock::now();
						auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - newSession->GetLastReadTime()).count();
						if(duration>=SERVER_SESSION_TIMEOUT_MS){
							newSession->CloseSession();
						} });
}

void ChatServer::HandleCloseConnection(const Connection::Ptr &conn)
{
	std::shared_ptr<Session> session;
	{
		std::lock_guard<std::mutex> lock(m_connIdSessionMtx);
		auto it = m_connIdToSession.find(conn->GetConnId());
		if (it != m_connIdToSession.end())
		{
			session = it->second.lock();
			m_connIdToSession.erase(it);
		}
	}
	if (session)
	{
		{
			std::lock_guard<std::mutex> lock(m_sessionMtx);
			auto it = m_sessionMap.find(session->GetSessionId());
			if (it != m_sessionMap.end())
			{
				m_sessionMap.erase(it);
			}
		}
		// 被动关闭session
		session->HandleOnCloseSession(conn);
	}
}

void ChatServer::HandleProtocolFrame(const Session::Ptr &session, const ProtocolFrame::Ptr &frame)
{
	if (m_isRunning)
	{
		m_servicePool.SubmitTask(
			[this](Session::Ptr sess, ProtocolFrame::Ptr frm)
			{
				try
				{
					sess->ResetLastReadTime();
					switch (frm->m_header.m_type)
					{
					case FrameType::PING:
					{
						auto f = std::make_shared<ProtocolFrame>(FrameType::PONG);
						sess->SendFrame(f);
						break;
					}
					case FrameType::REQUEST:
					{
						auto protocolResponse = ChatService::Instance().DispatchService(sess, frm);
						sess->SendFrame(protocolResponse);
						break;
					}

						// case FrameType::PUSH_ACK:
						// 	break;

					default:
						LOG_WARN("Unknown frame type {} from session {}", static_cast<uint8_t>(frm->m_header.m_type), sess->GetSessionId());
						break;
					}
				}
				catch (const std::exception &e)
				{
					LOG_ERROR("Exception handling frame for session {}: {}", sess->GetSessionId(), e.what());
				}
			},
			session, frame);
	}
}

void ChatServer::HandleCloseSession(const std::shared_ptr<Session> &session)
{
	AuthState state = session->GetAuthState();
	uint32_t userId = session->GetUser().m_id;
	if (state == AuthState::UnAUTHENTICATED || userId <= 0)
		return;
	auto &service = ChatService::Instance();
	service.RemoveUserSession(userId);
	// todo OnUserOffline
}