#pragma once
#include "gtest/gtest.h"
#include "ChatService.h"
#include "Utils.hpp"
#include "SQLiteCpp/SQLiteCpp.h"

class ChatServiceProtocolTest : public ::testing::Test
{
protected:
	ChatService &m_service = ChatService::Instance();
	const std::shared_ptr<SQLExecuser> &m_execuser = m_service.GetSQLExecuser();
	std::vector<User> m_users;
	std::vector<FriendRelation> m_friendRelations;
	std::vector<ChatRoom> m_chatRooms;
	std::vector<ChatRoomMember> m_chatroomMembers;
	std::vector<Message> m_messages;

	void SetUp() override
	{
		m_execuser->BeginTransaction();
		InsertTest();
	}

	void TearDown() override
	{
		m_execuser->RollbackTransaction();
	}

	void InsertTest()
	{
		m_users.push_back(m_execuser->ExecuteUpdateAndReturn<User>("INSERT INTO users (username, password) VALUES ('testuser', '" + Sha256("testpass") + "')"));
		m_users.push_back(m_execuser->ExecuteUpdateAndReturn<User>("INSERT INTO users (username, password) VALUES ('friendtest', '" + Sha256("testfriendpass") + "')"));
		m_users.push_back(m_execuser->ExecuteUpdateAndReturn<User>("INSERT INTO users (username, password) VALUES ('friendtest1', '" + Sha256("testfriendpass1") + "')"));
		m_users.push_back(m_execuser->ExecuteUpdateAndReturn<User>("INSERT INTO users (username, password) VALUES ('friendtest2', '" + Sha256("testfriendpass2") + "')"));
		m_users.push_back(m_execuser->ExecuteUpdateAndReturn<User>("INSERT INTO users (username, password) VALUES ('friendtest3', '" + Sha256("testfriendpass3") + "')"));
		m_friendRelations.push_back(m_execuser->ExecuteUpdateAndReturn<FriendRelation>(
			"INSERT INTO friends (user_id, friend_id, status) VALUES (" + std::to_string(m_users[0].m_id) + ", " + std::to_string(m_users[1].m_id) + ", " + " 1)"));
		m_friendRelations.push_back(m_execuser->ExecuteUpdateAndReturn<FriendRelation>(
			"INSERT INTO friends (user_id, friend_id, status) VALUES (" + std::to_string(m_users[1].m_id) + ", " + std::to_string(m_users[0].m_id) + ", " + " 1)"));
		m_friendRelations.push_back(m_execuser->ExecuteUpdateAndReturn<FriendRelation>(
			"INSERT INTO friends (user_id, friend_id, status) VALUES (" + std::to_string(m_users[0].m_id) + ", " + std::to_string(m_users[3].m_id) + ", " + " 1)"));
		m_friendRelations.push_back(m_execuser->ExecuteUpdateAndReturn<FriendRelation>(
			"INSERT INTO friends (user_id, friend_id, status) VALUES (" + std::to_string(m_users[3].m_id) + ", " + std::to_string(m_users[0].m_id) + ", " + " 1)"));
		m_friendRelations.push_back(m_execuser->ExecuteUpdateAndReturn<FriendRelation>(
			"INSERT INTO friends (user_id, friend_id, status) VALUES (" + std::to_string(m_users[4].m_id) + ", " + std::to_string(m_users[0].m_id) + ", " + " 0)"));
		m_friendRelations.push_back(m_execuser->ExecuteUpdateAndReturn<FriendRelation>(
			"INSERT INTO friends (user_id, friend_id, status) VALUES (" + std::to_string(m_users[2].m_id) + ", " + std::to_string(m_users[3].m_id) + ", " + " 0)"));
		m_chatRooms.push_back(m_execuser->ExecuteUpdateAndReturn<ChatRoom>(
			"INSERT INTO chat_rooms (name, owner_id) VALUES ('Test Room', " + std::to_string(m_users[0].m_id) + ")"));
		m_chatRooms.push_back(m_execuser->ExecuteUpdateAndReturn<ChatRoom>(
			"INSERT INTO chat_rooms (name, owner_id) VALUES ('Test Room', " + std::to_string(m_users[2].m_id) + ")"));
		m_chatRooms.push_back(m_execuser->ExecuteUpdateAndReturn<ChatRoom>(
			"INSERT INTO chat_rooms (name, owner_id) VALUES ('Test Room1', " + std::to_string(m_users[3].m_id) + ")"));
		m_chatroomMembers.push_back(m_execuser->ExecuteUpdateAndReturn<ChatRoomMember>(
			"INSERT INTO chat_room_members (room_id, user_id, status, role) VALUES (" + std::to_string(m_chatRooms[0].m_id) + ", " + std::to_string(m_users[0].m_id) + ", 1, 2)"));
		m_chatroomMembers.push_back(m_execuser->ExecuteUpdateAndReturn<ChatRoomMember>(
			"INSERT INTO chat_room_members (room_id, user_id, status, role) VALUES (" + std::to_string(m_chatRooms[0].m_id) + ", " + std::to_string(m_users[1].m_id) + ", 1, 2)"));
		m_chatroomMembers.push_back(m_execuser->ExecuteUpdateAndReturn<ChatRoomMember>(
			"INSERT INTO chat_room_members (room_id, user_id, status, role) VALUES (" + std::to_string(m_chatRooms[0].m_id) + ", " + std::to_string(m_users[2].m_id) + ", 1, 0)"));
		m_chatroomMembers.push_back(m_execuser->ExecuteUpdateAndReturn<ChatRoomMember>(
			"INSERT INTO chat_room_members (room_id, user_id, status, role) VALUES (" + std::to_string(m_chatRooms[1].m_id) + ", " + std::to_string(m_users[3].m_id) + ", 1, 2)"));
		m_chatroomMembers.push_back(m_execuser->ExecuteUpdateAndReturn<ChatRoomMember>(
			"INSERT INTO chat_room_members (room_id, user_id, status, role) VALUES (" + std::to_string(m_chatRooms[1].m_id) + ", " + std::to_string(m_users[1].m_id) + ", 1, 1)"));
		m_chatroomMembers.push_back(m_execuser->ExecuteUpdateAndReturn<ChatRoomMember>(
			"INSERT INTO chat_room_members (room_id, user_id, status, role) VALUES (" + std::to_string(m_chatRooms[2].m_id) + ", " + std::to_string(m_users[3].m_id) + ", 1, 1)"));
		m_chatroomMembers.push_back(m_execuser->ExecuteUpdateAndReturn<ChatRoomMember>(
			"INSERT INTO chat_room_members (room_id, user_id, inviter_id, status, role) VALUES (" + std::to_string(m_chatRooms[2].m_id) + ", " + std::to_string(m_users[0].m_id) + ", " + std::to_string(m_users[3].m_id) + ", 0, 1)"));
		m_messages.push_back(m_execuser->ExecuteUpdateAndReturn<Message>(
			"INSERT INTO messages (room_id, sender_id, message) VALUES (NULL, " + std::to_string(m_users[0].m_id) + ", " + " 'Test private message')"));
		m_messages.push_back(m_execuser->ExecuteUpdateAndReturn<Message>(
			"INSERT INTO messages (room_id, sender_id, message) VALUES (NULL, " + std::to_string(m_users[1].m_id) + ", " + " 'Test private message1')"));
		m_messages.push_back(m_execuser->ExecuteUpdateAndReturn<Message>(
			"INSERT INTO messages (room_id, sender_id, message) VALUES (" + std::to_string(m_chatRooms[0].m_id) + ", " + std::to_string(m_users[1].m_id) + ", " + " 'Test public message')"));
		m_messages.push_back(m_execuser->ExecuteUpdateAndReturn<Message>(
			"INSERT INTO messages (room_id, sender_id, message) VALUES (" + std::to_string(m_chatRooms[1].m_id) + ", " + std::to_string(m_users[2].m_id) + ", " + " 'Test public message')"));
		m_execuser->ExecuteUpdate(
			"INSERT INTO offline_recipients (message_id, recipient_id) VALUES (" + std::to_string(m_messages[0].m_id) + ", " + std::to_string(m_users[0].m_id) + ")");
		m_execuser->ExecuteUpdate(
			"INSERT INTO offline_recipients (message_id, recipient_id) VALUES (" + std::to_string(m_messages[1].m_id) + ", " + std::to_string(m_users[1].m_id) + ")");
		m_execuser->ExecuteUpdate(
			"INSERT INTO offline_recipients (message_id, recipient_id) VALUES (" + std::to_string(m_messages[2].m_id) + ", " + std::to_string(m_users[0].m_id) + ")");
	}
};

TEST_F(ChatServiceProtocolTest, RegisterHandler)
{
	auto request = std::make_shared<ProtocolFrame>();
	request->m_header.m_method = MethodType::REGISTER;
	request->m_header.m_type = FrameType::REQUEST;
	AuthRequest body;
	asio::io_context io_context;
	asio::ip::tcp::socket socket(io_context);
	auto conn = std::make_shared<Connection>(std::move(socket));
	conn->SetState(ConnectionState::CONNECTED);
	auto session = std::make_shared<Session>(conn);
	session->SetState(SessionState::ACTIVE);
	// EmptyUsernameOrPassword
	body.m_username = "";
	body.m_passwd = "any";
	request->m_payload = Serializer::Serialize<AuthRequest>(body);
	auto response = m_service.DispatchService(session, request);
	auto respBody = Serializer::DeSerialize<StatusResponse>(response->m_payload);
	EXPECT_EQ(response->m_header.m_status, Status::BAD_REQUEST);
	EXPECT_EQ(respBody.m_error_msg, "Username or passwd is empty!");
	body.m_username = "newuser";
	body.m_passwd = "";
	request->m_payload = Serializer::Serialize<AuthRequest>(body);
	response = m_service.DispatchService(session, request);
	respBody = Serializer::DeSerialize<StatusResponse>(response->m_payload);
	EXPECT_EQ(response->m_header.m_status, Status::BAD_REQUEST);
	EXPECT_EQ(respBody.m_error_msg, "Username or passwd is empty!");

	// UsernameExists
	body.m_username = "testuser";
	body.m_passwd = "somepass";
	request->m_payload = Serializer::Serialize<AuthRequest>(body);
	response = m_service.DispatchService(session, request);
	respBody = Serializer::DeSerialize<StatusResponse>(response->m_payload);
	EXPECT_EQ(response->m_header.m_status, Status::BAD_REQUEST);
	EXPECT_EQ(respBody.m_error_msg, "Username has been registered!");

	// Success
	body.m_username = "newuser";
	body.m_passwd = "testpass";
	request->m_payload = Serializer::Serialize<AuthRequest>(body);
	response = m_service.DispatchService(session, request);
	respBody = Serializer::DeSerialize<StatusResponse>(response->m_payload);
	EXPECT_EQ(response->m_header.m_status, Status::SUCCESS);
	EXPECT_TRUE(respBody.m_error_msg.empty());
	auto results = m_execuser->ExecuteQuery<User>("SELECT * from users WHERE users.username = ?", body.m_username);
	EXPECT_EQ(results.size(), 1);
}

class UserLoginAndLogout
{
public:
	UserLoginAndLogout(const Session::Ptr &session, const std::string &username, const std::string &password)
		: m_session(session)
	{
		m_user = UserLogin(m_session, username, password);
	}

	~UserLoginAndLogout()
	{
		UserLogout(m_session, m_user.m_id);
	}

	static User UserLogin(const Session::Ptr &session, const std::string &username, const std::string &password)
	{
		auto request = std::make_shared<ProtocolFrame>();
		AuthRequest login_body;
		request->m_header.m_method = MethodType::LOGIN;
		request->m_header.m_type = FrameType::REQUEST;
		login_body.m_username = username;
		login_body.m_passwd = password;
		request->m_payload = Serializer::Serialize<AuthRequest>(login_body);
		auto response = ChatService::Instance().DispatchService(session, request);
		EXPECT_EQ(response->m_header.m_status, Status::SUCCESS);
		auto respBody = Serializer::DeSerialize<AuthResponse>(response->m_payload);
		return respBody.m_user;
	}

	static void UserLogout(const Session::Ptr &session, uint32_t user_id)
	{
		auto request = std::make_shared<ProtocolFrame>();
		UserIdRequest login_body;
		login_body.m_userId = user_id;
		request->m_header.m_method = MethodType::LOGOUT;
		request->m_payload = Serializer::Serialize<UserIdRequest>(login_body);
		auto response = ChatService::Instance().DispatchService(session, request);
	}

private:
	Session::Ptr m_session;
	User m_user;
};

TEST_F(ChatServiceProtocolTest, LoginHandler)
{
	auto request = std::make_shared<ProtocolFrame>();
	request->m_header.m_method = MethodType::LOGIN;
	request->m_header.m_type = FrameType::REQUEST;
	AuthRequest body;
	asio::io_context io_context;
	asio::ip::tcp::socket socket(io_context);
	auto conn = std::make_shared<Connection>(std::move(socket));
	conn->SetState(ConnectionState::CONNECTED);
	auto session = std::make_shared<Session>(conn);
	session->SetState(SessionState::ACTIVE);

	// EmptyUsernameOrPassword
	body.m_username = "";
	body.m_passwd = "any";
	request->m_payload = Serializer::Serialize<AuthRequest>(body);
	auto response = m_service.DispatchService(session, request);
	auto respBody = Serializer::DeSerialize<AuthResponse>(response->m_payload);
	EXPECT_EQ(response->m_header.m_status, Status::BAD_REQUEST);
	EXPECT_EQ(respBody.m_error_msg, "Username or passwd is empty!");
	body.m_username = "newuser";
	body.m_passwd = "";
	request->m_payload = Serializer::Serialize<AuthRequest>(body);
	response = m_service.DispatchService(session, request);
	respBody = Serializer::DeSerialize<AuthResponse>(response->m_payload);
	EXPECT_EQ(response->m_header.m_status, Status::BAD_REQUEST);
	EXPECT_EQ(respBody.m_error_msg, "Username or passwd is empty!");

	// UserNotRegistered
	body.m_username = "nonexistent";
	body.m_passwd = "any";
	request->m_payload = Serializer::Serialize<AuthRequest>(body);
	response = m_service.DispatchService(session, request);
	respBody = Serializer::DeSerialize<AuthResponse>(response->m_payload);
	EXPECT_EQ(response->m_header.m_status, Status::BAD_REQUEST);
	EXPECT_EQ(respBody.m_error_msg, "Username has not registered!");

	// PasswordError
	body.m_username = "testuser";
	body.m_passwd = "wrongpasshash";
	request->m_payload = Serializer::Serialize<AuthRequest>(body);
	response = m_service.DispatchService(session, request);
	respBody = Serializer::DeSerialize<AuthResponse>(response->m_payload);
	EXPECT_EQ(response->m_header.m_status, Status::BAD_REQUEST);
	EXPECT_EQ(respBody.m_error_msg, "Passwd error!");

	// Status::SUCCESS
	body.m_username = "testuser";
	body.m_passwd = "testpass";
	request->m_payload = Serializer::Serialize<AuthRequest>(body);
	response = m_service.DispatchService(session, request);
	respBody = Serializer::DeSerialize<AuthResponse>(response->m_payload);
	EXPECT_EQ(response->m_header.m_status, Status::SUCCESS);
	EXPECT_EQ(respBody.m_friends.size(), 2);
	EXPECT_EQ(respBody.m_offlineMessages.size(), 2);
	EXPECT_EQ(respBody.m_chatRooms.size(), 1);
	EXPECT_EQ(respBody.m_chatRoomInvitations.size(), 1);
	EXPECT_EQ(respBody.m_friendInvitations.size(), 1);
	// UserAlreadyOnline
	response = m_service.DispatchService(session, request);
	respBody = Serializer::DeSerialize<AuthResponse>(response->m_payload);
	EXPECT_EQ(response->m_header.m_status, Status::BAD_REQUEST);
	EXPECT_EQ(respBody.m_error_msg, "User has online!");
	// reset
	UserLoginAndLogout::UserLogout(session, m_users[0].m_id);
}

TEST_F(ChatServiceProtocolTest, LogoutHandler)
{
	auto request = std::make_shared<ProtocolFrame>();
	request->m_header.m_method = MethodType::LOGOUT;
	UserIdRequest body;
	asio::io_context io_context;
	asio::ip::tcp::socket socket(io_context);
	auto conn = std::make_shared<Connection>(std::move(socket));
	conn->SetState(ConnectionState::CONNECTED);
	auto session = std::make_shared<Session>(conn);
	session->SetState(SessionState::ACTIVE);
	// Unauthorized
	body.m_userId = m_users[0].m_id + 1;
	request->m_payload = Serializer::Serialize<UserIdRequest>(body);
	auto response = m_service.DispatchService(session, request);
	auto respBody = Serializer::DeSerialize<StatusResponse>(response->m_payload);
	EXPECT_EQ(response->m_header.m_status, Status::UNAUTHORIZED);
	EXPECT_EQ(respBody.m_error_msg, "Unauthorized access!");
	// Status::SUCCESS
	UserLoginAndLogout userLogin(session, "testuser", "testpass");
	body.m_userId = m_users[0].m_id;
	request->m_header.m_method = MethodType::LOGOUT;
	request->m_payload = Serializer::Serialize<UserIdRequest>(body);
	response = m_service.DispatchService(session, request);
	respBody = Serializer::DeSerialize<StatusResponse>(response->m_payload);
	EXPECT_EQ(response->m_header.m_status, Status::SUCCESS);
	EXPECT_TRUE(respBody.m_error_msg.empty());
}

TEST_F(ChatServiceProtocolTest, CreateChatRoomHandler)
{
	auto request = std::make_shared<ProtocolFrame>();
	request->m_header.m_method = MethodType::CREATE_CHAT_ROOM;
	CreateChatRoomRequest body;
	asio::io_context io_context;
	asio::ip::tcp::socket socket(io_context);
	auto conn = std::make_shared<Connection>(std::move(socket));
	conn->SetState(ConnectionState::CONNECTED);
	auto session = std::make_shared<Session>(conn);
	session->SetState(SessionState::ACTIVE);
	UserLoginAndLogout userLogin(session, "testuser", "testpass");
	// Unauthorized
	body.m_ownerId = m_users[0].m_id + 1;
	body.m_name = "New Room";
	request->m_payload = Serializer::Serialize<CreateChatRoomRequest>(body);
	auto response = m_service.DispatchService(session, request);
	auto respBody = Serializer::DeSerialize<ChatRoomResponse>(response->m_payload);
	EXPECT_EQ(response->m_header.m_status, Status::UNAUTHORIZED);
	EXPECT_EQ(respBody.m_error_msg, "Unauthorized access!");
	// EmptyName
	body.m_ownerId = m_users[0].m_id;
	body.m_name = "";
	request->m_payload = Serializer::Serialize<CreateChatRoomRequest>(body);
	response = m_service.DispatchService(session, request);
	respBody = Serializer::DeSerialize<ChatRoomResponse>(response->m_payload);
	EXPECT_EQ(response->m_header.m_status, Status::BAD_REQUEST);
	EXPECT_EQ(respBody.m_error_msg, "Chat room name is empty!");
	// Status::SUCCESS
	body.m_name = "New Room";
	request->m_payload = Serializer::Serialize<CreateChatRoomRequest>(body);
	response = m_service.DispatchService(session, request);
	respBody = Serializer::DeSerialize<ChatRoomResponse>(response->m_payload);
	EXPECT_EQ(response->m_header.m_status, Status::SUCCESS);
	EXPECT_TRUE(respBody.m_error_msg.empty());
	EXPECT_EQ(respBody.m_chatRoom.m_name, body.m_name);
	EXPECT_EQ(respBody.m_chatRoom.m_ownerId, body.m_ownerId);
}

TEST_F(ChatServiceProtocolTest, InviteToChatRoomHandler)
{
	auto request = std::make_shared<ProtocolFrame>();
	request->m_header.m_method = MethodType::INVITE_TO_CHAT_ROOM;
	InviteToChatRoomRequest body;
	asio::io_context io_context;
	asio::ip::tcp::socket socket1(io_context);
	auto conn = std::make_shared<Connection>(std::move(socket1));
	conn->SetState(ConnectionState::CONNECTED);
	auto session = std::make_shared<Session>(conn);
	session->SetState(SessionState::ACTIVE);
	UserLoginAndLogout userLogin1(session, "testuser", "testpass");
	asio::ip::tcp::socket socket2(io_context);
	auto frdConn = std::make_shared<Connection>(std::move(socket2));
	frdConn->SetState(ConnectionState::CONNECTED);
	auto frdSession = std::make_shared<Session>(frdConn);
	frdSession->SetState(SessionState::ACTIVE);
	UserLoginAndLogout userLogin2(frdSession, "friendtest3", "testfriendpass3");
	// Unauthorized
	body.m_inviterId = m_users[0].m_id + 1;
	body.m_roomId = m_chatRooms[0].m_id;
	body.m_inviteeId = m_users[1].m_id;
	request->m_payload = Serializer::Serialize<InviteToChatRoomRequest>(body);
	auto response = m_service.DispatchService(session, request);
	auto respBody = Serializer::DeSerialize<ChatRoomInvitationResponse>(response->m_payload);
	EXPECT_EQ(response->m_header.m_status, Status::UNAUTHORIZED);
	EXPECT_EQ(respBody.m_error_msg, "Unauthorized access!");
	// UserAlreadyInChatRoom
	body.m_inviterId = m_users[0].m_id;
	body.m_roomId = m_chatRooms[0].m_id;
	body.m_inviteeId = m_users[1].m_id;
	request->m_payload = Serializer::Serialize<InviteToChatRoomRequest>(body);
	response = m_service.DispatchService(session, request);
	respBody = Serializer::DeSerialize<ChatRoomInvitationResponse>(response->m_payload);
	EXPECT_EQ(response->m_header.m_status, Status::BAD_REQUEST);
	EXPECT_EQ(respBody.m_error_msg, "User has already in chat room!");
	// Status::SUCCESS
	body.m_inviteeId = m_users[4].m_id;
	request->m_payload = Serializer::Serialize<InviteToChatRoomRequest>(body);
	response = m_service.DispatchService(session, request);
	respBody = Serializer::DeSerialize<ChatRoomInvitationResponse>(response->m_payload);
	EXPECT_EQ(response->m_header.m_status, Status::SUCCESS);
	EXPECT_TRUE(respBody.m_error_msg.empty());
	std::string responseStr = frdConn->GetSendQueue().front();
	frdConn->GetSendQueue().pop();
	auto pushResp = ProtocolCodec().UnPackProtocolFrame(std::string(responseStr.begin(), responseStr.end()).data(), responseStr.size())[0];
	auto inviteBody = Serializer::DeSerialize<ChatRoomInvitation>(pushResp->m_payload);
	EXPECT_EQ(pushResp->m_header.m_method, MethodType::PUSH_CHAT_ROOM_INVITATION);
	EXPECT_EQ(pushResp->m_header.m_type, FrameType::PUSH);
	EXPECT_EQ(pushResp->m_header.m_requestId, 0);
	EXPECT_EQ(pushResp->m_header.m_status, Status::SUCCESS);
	EXPECT_EQ(inviteBody.m_id, respBody.m_invitation.m_id);
	EXPECT_EQ(inviteBody.m_inviter.m_id, m_users[0].m_id);
	EXPECT_EQ(inviteBody.m_inviter.m_username, m_users[0].m_username);
	EXPECT_EQ(inviteBody.m_chatRoom.m_id, m_chatRooms[0].m_id);
	EXPECT_EQ(inviteBody.m_chatRoom.m_name, m_chatRooms[0].m_name);
	EXPECT_EQ(inviteBody.m_chatRoom.m_ownerId, m_users[0].m_id);
}

TEST_F(ChatServiceProtocolTest, AcceptChatRoomInvitationHandler)
{
	auto request = std::make_shared<ProtocolFrame>();
	request->m_header.m_method = MethodType::ACCEPT_CHAT_ROOM_INVITATION;
	HandleInvitationRequest body;
	asio::io_context io_context;
	asio::ip::tcp::socket socket1(io_context);
	auto conn = std::make_shared<Connection>(std::move(socket1));
	conn->SetState(ConnectionState::CONNECTED);
	auto session = std::make_shared<Session>(std::move(conn));
	session->SetState(SessionState::ACTIVE);
	UserLoginAndLogout userLogin1(session, "testuser", "testpass");
	asio::ip::tcp::socket socket2(io_context);
	auto frdConn = std::make_shared<Connection>(std::move(socket2));
	frdConn->SetState(ConnectionState::CONNECTED);
	auto frdSession = std::make_shared<Session>(frdConn);
	frdSession->SetState(SessionState::ACTIVE);
	UserLoginAndLogout userLogin2(frdSession, "friendtest2", "testfriendpass2");
	// Unauthorized
	body.m_userId = m_users[0].m_id + 1;
	body.m_invitation_id = m_chatroomMembers[6].m_id;
	request->m_payload = Serializer::Serialize<HandleInvitationRequest>(body);
	auto response = m_service.DispatchService(session, request);
	auto respBody = Serializer::DeSerialize<ChatRoomResponse>(response->m_payload);
	EXPECT_EQ(response->m_header.m_status, Status::UNAUTHORIZED);
	EXPECT_EQ(respBody.m_error_msg, "Unauthorized access!");
	// InvitationNotFound
	body.m_userId = m_users[0].m_id;
	body.m_invitation_id = m_chatroomMembers[0].m_id;
	request->m_payload = Serializer::Serialize<HandleInvitationRequest>(body);
	response = m_service.DispatchService(session, request);
	respBody = Serializer::DeSerialize<ChatRoomResponse>(response->m_payload);
	EXPECT_EQ(response->m_header.m_status, Status::BAD_REQUEST);
	EXPECT_EQ(respBody.m_error_msg, "Invitation not found!");
	// Status::SUCCESS
	body.m_invitation_id = m_chatroomMembers[6].m_id;
	request->m_payload = Serializer::Serialize<HandleInvitationRequest>(body);
	response = m_service.DispatchService(session, request);
	respBody = Serializer::DeSerialize<ChatRoomResponse>(response->m_payload);
	EXPECT_EQ(response->m_header.m_status, Status::SUCCESS);
	EXPECT_TRUE(respBody.m_error_msg.empty());
	EXPECT_EQ(respBody.m_chatRoom.m_id, m_chatRooms[2].m_id);
	std::string responseStr = frdConn->GetSendQueue().front();
	frdConn->GetSendQueue().pop();
	auto pushResp = ProtocolCodec().UnPackProtocolFrame(std::string(responseStr.begin(), responseStr.end()).data(), responseStr.size())[0];
	auto inviteDecBody = Serializer::DeSerialize<ChatRoomInvitationDecision>(pushResp->m_payload);
	EXPECT_EQ(pushResp->m_header.m_method, MethodType::PUSH_CHAT_ROOM_INVITATION_DECISION);
	EXPECT_EQ(pushResp->m_header.m_type, FrameType::PUSH);
	EXPECT_EQ(pushResp->m_header.m_requestId, 0);
	EXPECT_EQ(pushResp->m_header.m_status, Status::SUCCESS);
	EXPECT_EQ(inviteDecBody.m_invitationId, m_chatroomMembers[6].m_id);
	EXPECT_EQ(inviteDecBody.m_chatRoom.m_id, m_chatRooms[2].m_id);
	EXPECT_EQ(inviteDecBody.m_chatRoom.m_name, m_chatRooms[2].m_name);
	EXPECT_EQ(inviteDecBody.m_chatRoom.m_ownerId, m_chatRooms[2].m_ownerId);
	EXPECT_EQ(inviteDecBody.m_status, AddRelationStatus::ACCEPTED);
}

TEST_F(ChatServiceProtocolTest, RejectChatRoomInvitationHandler)
{
	auto request = std::make_shared<ProtocolFrame>();
	request->m_header.m_method = MethodType::REJECT_CHAT_ROOM_INVITATION;
	HandleInvitationRequest body;
	asio::io_context io_context;
	asio::ip::tcp::socket socket1(io_context);
	auto conn = std::make_shared<Connection>(std::move(socket1));
	conn->SetState(ConnectionState::CONNECTED);
	auto session = std::make_shared<Session>(conn);
	session->SetState(SessionState::ACTIVE);
	UserLoginAndLogout userLogin1(session, "testuser", "testpass");
	asio::ip::tcp::socket socket2(io_context);
	auto frdConn = std::make_shared<Connection>(std::move(socket2));
	frdConn->SetState(ConnectionState::CONNECTED);
	auto frdSession = std::make_shared<Session>(frdConn);
	frdSession->SetState(SessionState::ACTIVE);
	UserLoginAndLogout userLogin2(frdSession, "friendtest2", "testfriendpass2");
	// Unauthorized
	body.m_userId = m_users[0].m_id + 1;
	body.m_invitation_id = m_chatroomMembers[6].m_id;
	request->m_payload = Serializer::Serialize<HandleInvitationRequest>(body);
	auto response = m_service.DispatchService(session, request);
	auto respBody = Serializer::DeSerialize<StatusResponse>(response->m_payload);
	EXPECT_EQ(response->m_header.m_status, Status::UNAUTHORIZED);
	EXPECT_EQ(respBody.m_error_msg, "Unauthorized access!");
	// InvitationNotFound
	body.m_userId = m_users[0].m_id;
	body.m_invitation_id = m_chatroomMembers[0].m_id;
	request->m_payload = Serializer::Serialize<HandleInvitationRequest>(body);
	response = m_service.DispatchService(session, request);
	respBody = Serializer::DeSerialize<StatusResponse>(response->m_payload);
	EXPECT_EQ(response->m_header.m_status, Status::BAD_REQUEST);
	EXPECT_EQ(respBody.m_error_msg, "Invitation not found!");
	// Status::SUCCESS
	body.m_invitation_id = m_chatroomMembers[6].m_id;
	request->m_payload = Serializer::Serialize<HandleInvitationRequest>(body);
	response = m_service.DispatchService(session, request);
	respBody = Serializer::DeSerialize<StatusResponse>(response->m_payload);
	std::string responseStr = frdConn->GetSendQueue().front();
	frdConn->GetSendQueue().pop();
	auto pushResp = ProtocolCodec().UnPackProtocolFrame(std::string(responseStr.begin(), responseStr.end()).data(), responseStr.size())[0];
	auto inviteDecBody = Serializer::DeSerialize<ChatRoomInvitationDecision>(pushResp->m_payload);
	EXPECT_EQ(response->m_header.m_status, Status::SUCCESS);
	EXPECT_TRUE(respBody.m_error_msg.empty());
	EXPECT_EQ(pushResp->m_header.m_method, MethodType::PUSH_CHAT_ROOM_INVITATION_DECISION);
	EXPECT_EQ(pushResp->m_header.m_type, FrameType::PUSH);
	EXPECT_EQ(pushResp->m_header.m_status, Status::SUCCESS);
	EXPECT_EQ(inviteDecBody.m_invitationId, m_chatroomMembers[6].m_id);
	EXPECT_EQ(inviteDecBody.m_chatRoom.m_id, m_chatRooms[2].m_id);
	EXPECT_EQ(inviteDecBody.m_chatRoom.m_name, m_chatRooms[2].m_name);
	EXPECT_EQ(inviteDecBody.m_chatRoom.m_ownerId, m_chatRooms[2].m_ownerId);
	EXPECT_EQ(inviteDecBody.m_status, AddRelationStatus::REJECTED);
}

TEST_F(ChatServiceProtocolTest, OneChatHandler)
{
	auto request = std::make_shared<ProtocolFrame>();
	request->m_header.m_method = MethodType::ONE_CHAT;
	OneChatRequest body;
	asio::io_context io_context;
	asio::ip::tcp::socket socket1(io_context);
	auto conn = std::make_shared<Connection>(std::move(socket1));
	conn->SetState(ConnectionState::CONNECTED);
	auto session = std::make_shared<Session>(conn);
	session->SetState(SessionState::ACTIVE);
	UserLoginAndLogout userLogin1(session, "testuser", "testpass");
	asio::ip::tcp::socket socket2(io_context);
	auto frdConn = std::make_shared<Connection>(std::move(socket2));
	frdConn->SetState(ConnectionState::CONNECTED);
	auto frdSession = std::make_shared<Session>(frdConn);
	frdSession->SetState(SessionState::ACTIVE);
	UserLoginAndLogout userLogin2(frdSession, "friendtest", "testfriendpass");
	// Unauthorized
	body.m_message = "Hello, friend!";
	body.m_receiverId = m_users[1].m_id;
	body.m_senderId = m_users[0].m_id + 1;
	request->m_payload = Serializer::Serialize<OneChatRequest>(body);
	auto response = m_service.DispatchService(session, request);
	auto respBody = Serializer::DeSerialize<MessageResponse>(response->m_payload);
	EXPECT_EQ(response->m_header.m_status, Status::UNAUTHORIZED);
	EXPECT_EQ(respBody.m_error_msg, "Unauthorized access!");
	// Send to self
	body.m_receiverId = m_users[0].m_id;
	body.m_senderId = m_users[0].m_id;
	request->m_payload = Serializer::Serialize<OneChatRequest>(body);
	response = m_service.DispatchService(session, request);
	respBody = Serializer::DeSerialize<MessageResponse>(response->m_payload);
	EXPECT_EQ(response->m_header.m_status, Status::BAD_REQUEST);
	EXPECT_EQ(respBody.m_error_msg, "Cannot send message to self!");
	// Message empty
	body.m_message = "";
	body.m_receiverId = m_users[1].m_id;
	request->m_payload = Serializer::Serialize<OneChatRequest>(body);
	response = m_service.DispatchService(session, request);
	respBody = Serializer::DeSerialize<MessageResponse>(response->m_payload);
	EXPECT_EQ(response->m_header.m_status, Status::BAD_REQUEST);
	EXPECT_EQ(respBody.m_error_msg, "Message is empty!");
	// Success
	body.m_message = "Hello, friend!";
	request->m_payload = Serializer::Serialize<OneChatRequest>(body);
	response = m_service.DispatchService(session, request);
	respBody = Serializer::DeSerialize<MessageResponse>(response->m_payload);
	std::string responseStr = frdConn->GetSendQueue().front();
	frdConn->GetSendQueue().pop();
	auto pushResp = ProtocolCodec().UnPackProtocolFrame(std::string(responseStr.begin(), responseStr.end()).data(), responseStr.size())[0];
	auto messageBody = Serializer::DeSerialize<Message>(pushResp->m_payload);
	EXPECT_EQ(response->m_header.m_status, Status::SUCCESS);
	EXPECT_TRUE(respBody.m_error_msg.empty());
	EXPECT_EQ(pushResp->m_header.m_method, MethodType::PUSH_MESSAGE);
	EXPECT_EQ(pushResp->m_header.m_type, FrameType::PUSH);
	EXPECT_EQ(messageBody.m_senderId, m_users[0].m_id);
	EXPECT_EQ(messageBody.m_roomId, 0);
	EXPECT_EQ(messageBody.m_message, "Hello, friend!");
	EXPECT_EQ(messageBody.m_id, respBody.m_message.m_id);
	EXPECT_EQ(messageBody.m_createdAt, respBody.m_message.m_createdAt);
}

TEST_F(ChatServiceProtocolTest, GroupChatHandler)
{
	auto request = std::make_shared<ProtocolFrame>();
	request->m_header.m_method = MethodType::GROUP_CHAT;
	GroupChatRequest body;
	asio::io_context io_context;
	asio::ip::tcp::socket socket1(io_context);
	auto conn = std::make_shared<Connection>(std::move(socket1));
	conn->SetState(ConnectionState::CONNECTED);
	auto session = std::make_shared<Session>(conn);
	session->SetState(SessionState::ACTIVE);
	UserLoginAndLogout userLogin1(session, "testuser", "testpass");
	asio::ip::tcp::socket socket2(io_context);
	auto frd1Conn = std::make_shared<Connection>(std::move(socket2));
	frd1Conn->SetState(ConnectionState::CONNECTED);
	auto frd1Session = std::make_shared<Session>(frd1Conn);
	frd1Session->SetState(SessionState::ACTIVE);
	UserLoginAndLogout userLogin2(frd1Session, "friendtest", "testfriendpass");
	asio::ip::tcp::socket socket3(io_context);
	auto frd2Conn = std::make_shared<Connection>(std::move(socket3));
	frd2Conn->SetState(ConnectionState::CONNECTED);
	auto frd2Session = std::make_shared<Session>(frd2Conn);
	frd2Session->SetState(SessionState::ACTIVE);
	UserLoginAndLogout userLogin3(frd2Session, "friendtest1", "testfriendpass1");
	// Unauthorized
	body.m_message = "Hello, friends!";
	body.m_roomId = m_chatRooms[0].m_id;
	body.m_senderId = m_users[0].m_id + 1;
	request->m_payload = Serializer::Serialize<GroupChatRequest>(body);
	auto response = m_service.DispatchService(session, request);
	auto respBody = Serializer::DeSerialize<MessageResponse>(response->m_payload);
	EXPECT_EQ(response->m_header.m_status, Status::UNAUTHORIZED);
	EXPECT_EQ(respBody.m_error_msg, "Unauthorized access!");
	// Message empty
	body.m_message = "";
	body.m_senderId = m_users[0].m_id;
	request->m_payload = Serializer::Serialize<GroupChatRequest>(body);
	response = m_service.DispatchService(session, request);
	respBody = Serializer::DeSerialize<MessageResponse>(response->m_payload);
	EXPECT_EQ(response->m_header.m_status, Status::BAD_REQUEST);
	EXPECT_EQ(respBody.m_error_msg, "Message is empty!");
	// Success
	body.m_message = "Hello, friends!";
	request->m_payload = Serializer::Serialize<GroupChatRequest>(body);
	response = m_service.DispatchService(session, request);
	respBody = Serializer::DeSerialize<MessageResponse>(response->m_payload);
	std::string responseStr = frd1Conn->GetSendQueue().front();
	frd1Conn->GetSendQueue().pop();
	auto pushResp1 = ProtocolCodec().UnPackProtocolFrame(std::string(responseStr.begin(), responseStr.end()).data(), responseStr.size())[0];
	auto messageBody1 = Serializer::DeSerialize<Message>(pushResp1->m_payload);
	responseStr = frd2Conn->GetSendQueue().front();
	frd2Conn->GetSendQueue().pop();
	auto pushResp2 = ProtocolCodec().UnPackProtocolFrame(std::string(responseStr.begin(), responseStr.end()).data(), responseStr.size())[0];
	auto messageBody2 = Serializer::DeSerialize<Message>(pushResp2->m_payload);
	EXPECT_EQ(response->m_header.m_status, Status::SUCCESS);
	EXPECT_TRUE(respBody.m_error_msg.empty());
	EXPECT_EQ(pushResp1->m_header.m_method, MethodType::PUSH_MESSAGE);
	EXPECT_EQ(pushResp1->m_header.m_type, FrameType::PUSH);
	EXPECT_EQ(pushResp1->m_header.m_status, Status::SUCCESS);
	EXPECT_EQ(messageBody1.m_id, respBody.m_message.m_id);
	EXPECT_EQ(messageBody1.m_roomId, m_chatRooms[0].m_id);
	EXPECT_EQ(messageBody1.m_senderId, m_users[0].m_id);
	EXPECT_EQ(messageBody1.m_message, "Hello, friends!");
	EXPECT_EQ(pushResp2->m_header.m_method, MethodType::PUSH_MESSAGE);
	EXPECT_EQ(pushResp2->m_header.m_type, FrameType::PUSH);
	EXPECT_EQ(pushResp2->m_header.m_status, Status::SUCCESS);
	EXPECT_EQ(messageBody2.m_id, respBody.m_message.m_id);
	EXPECT_EQ(messageBody2.m_roomId, m_chatRooms[0].m_id);
	EXPECT_EQ(messageBody2.m_senderId, m_users[0].m_id);
	EXPECT_EQ(messageBody2.m_message, "Hello, friends!");
}

TEST_F(ChatServiceProtocolTest, AddFriendHandler)
{
	auto request = std::make_shared<ProtocolFrame>();
	request->m_header.m_method = MethodType::ADD_FRIEND;
	AddFriendRequest body;
	asio::io_context io_context;
	asio::ip::tcp::socket socket1(io_context);
	auto conn = std::make_shared<Connection>(std::move(socket1));
	conn->SetState(ConnectionState::CONNECTED);
	auto session = std::make_shared<Session>(conn);
	session->SetState(SessionState::ACTIVE);
	UserLoginAndLogout userLogin1(session, "testuser", "testpass");
	asio::ip::tcp::socket socket2(io_context);
	auto frdConn = std::make_shared<Connection>(std::move(socket2));
	frdConn->SetState(ConnectionState::CONNECTED);
	auto frdSession = std::make_shared<Session>(frdConn);
	frdSession->SetState(SessionState::ACTIVE);
	UserLoginAndLogout userLogin2(frdSession, "friendtest1", "testfriendpass1");
	// Unauthorized
	body.m_userId = m_users[0].m_id + 1;
	body.m_friendId = m_users[2].m_id;
	request->m_payload = Serializer::Serialize<AddFriendRequest>(body);
	auto response = m_service.DispatchService(session, request);
	auto respBody = Serializer::DeSerialize<FriendInvitationResponse>(response->m_payload);
	EXPECT_EQ(response->m_header.m_status, Status::UNAUTHORIZED);
	EXPECT_EQ(respBody.m_error_msg, "Unauthorized access!");
	// Add self
	body.m_userId = m_users[0].m_id;
	body.m_friendId = m_users[0].m_id;
	request->m_payload = Serializer::Serialize<AddFriendRequest>(body);
	response = m_service.DispatchService(session, request);
	respBody = Serializer::DeSerialize<FriendInvitationResponse>(response->m_payload);
	EXPECT_EQ(response->m_header.m_status, Status::BAD_REQUEST);
	EXPECT_EQ(respBody.m_error_msg, "Cannot add self as friend!");
	// Already friends
	body.m_friendId = m_users[1].m_id;
	request->m_payload = Serializer::Serialize<AddFriendRequest>(body);
	response = m_service.DispatchService(session, request);
	respBody = Serializer::DeSerialize<FriendInvitationResponse>(response->m_payload);
	EXPECT_EQ(response->m_header.m_status, Status::BAD_REQUEST);
	EXPECT_EQ(respBody.m_error_msg, "Friend already exists!");
	// Request is pending
	body.m_friendId = m_users[4].m_id;
	request->m_payload = Serializer::Serialize<AddFriendRequest>(body);
	response = m_service.DispatchService(session, request);
	respBody = Serializer::DeSerialize<FriendInvitationResponse>(response->m_payload);
	EXPECT_EQ(response->m_header.m_status, Status::BAD_REQUEST);
	EXPECT_EQ(respBody.m_error_msg, "Friend request is pending!");
	// Status::SUCCESS
	body.m_friendId = m_users[2].m_id;
	request->m_payload = Serializer::Serialize<AddFriendRequest>(body);
	response = m_service.DispatchService(session, request);
	respBody = Serializer::DeSerialize<FriendInvitationResponse>(response->m_payload);
	EXPECT_EQ(response->m_header.m_status, Status::SUCCESS);
	EXPECT_TRUE(respBody.m_error_msg.empty());
	std::string responseStr = frdConn->GetSendQueue().front();
	frdConn->GetSendQueue().pop();
	auto pushResp = ProtocolCodec().UnPackProtocolFrame(std::string(responseStr.begin(), responseStr.end()).data(), responseStr.size())[0];
	auto inviteBody = Serializer::DeSerialize<FriendInvitation>(pushResp->m_payload);
	EXPECT_EQ(pushResp->m_header.m_method, MethodType::PUSH_FRIEND_INVITATION);
	EXPECT_EQ(pushResp->m_header.m_type, FrameType::PUSH);
	EXPECT_EQ(pushResp->m_header.m_status, Status::SUCCESS);
	EXPECT_EQ(inviteBody.m_id, respBody.m_invitation.m_id);
	EXPECT_EQ(inviteBody.m_inviter.m_id, m_users[0].m_id);
	EXPECT_EQ(inviteBody.m_inviter.m_username, m_users[0].m_username);
}

TEST_F(ChatServiceProtocolTest, AcceptFriendHandler)
{
	auto request = std::make_shared<ProtocolFrame>();
	request->m_header.m_method = MethodType::ACCEPT_FRIEND_INVITATION;
	HandleInvitationRequest body;
	asio::io_context io_context;
	asio::ip::tcp::socket socket1(io_context);
	auto conn = std::make_shared<Connection>(std::move(socket1));
	conn->SetState(ConnectionState::CONNECTED);
	auto session = std::make_shared<Session>(conn);
	session->SetState(SessionState::ACTIVE);
	UserLoginAndLogout userLogin1(session, "testuser", "testpass");
	asio::ip::tcp::socket socket2(io_context);
	auto frdConn = std::make_shared<Connection>(std::move(socket2));
	frdConn->SetState(ConnectionState::CONNECTED);
	auto frdSession = std::make_shared<Session>(frdConn);
	frdSession->SetState(SessionState::ACTIVE);
	UserLoginAndLogout userLogin2(frdSession, "friendtest3", "testfriendpass3");
	// Unauthorized
	body.m_userId = m_users[0].m_id + 1;
	body.m_invitation_id = m_friendRelations[4].m_id;
	request->m_payload = Serializer::Serialize<HandleInvitationRequest>(body);
	auto response = m_service.DispatchService(session, request);
	auto respBody = Serializer::DeSerialize<UserResponse>(response->m_payload);
	EXPECT_EQ(response->m_header.m_status, Status::UNAUTHORIZED);
	EXPECT_EQ(respBody.m_error_msg, "Unauthorized access!");
	// InvitationNotFound
	body.m_userId = m_users[0].m_id;
	body.m_invitation_id = m_friendRelations[5].m_id;
	request->m_payload = Serializer::Serialize<HandleInvitationRequest>(body);
	response = m_service.DispatchService(session, request);
	respBody = Serializer::DeSerialize<UserResponse>(response->m_payload);
	EXPECT_EQ(response->m_header.m_status, Status::BAD_REQUEST);
	EXPECT_EQ(respBody.m_error_msg, "Invitation not found!");
	// Status::SUCCESS
	body.m_invitation_id = m_friendRelations[4].m_id;
	request->m_payload = Serializer::Serialize<HandleInvitationRequest>(body);
	response = m_service.DispatchService(session, request);
	respBody = Serializer::DeSerialize<UserResponse>(response->m_payload);
	std::string responseStr = frdConn->GetSendQueue().front();
	frdConn->GetSendQueue().pop();
	auto pushResp = ProtocolCodec().UnPackProtocolFrame(std::string(responseStr.begin(), responseStr.end()).data(), responseStr.size())[0];
	auto inviteDecBody = Serializer::DeSerialize<FriendInvitationDecision>(pushResp->m_payload);
	EXPECT_EQ(response->m_header.m_status, Status::SUCCESS);
	EXPECT_TRUE(respBody.m_error_msg.empty());
	EXPECT_EQ(respBody.m_user.m_id, m_users[4].m_id);
	EXPECT_EQ(respBody.m_user.m_username, m_users[4].m_username);
	EXPECT_EQ(respBody.m_user.m_state, UserState::ONLINE);
	EXPECT_EQ(pushResp->m_header.m_method, MethodType::PUSH_FRIEND_INVITATION_DECISION);
	EXPECT_EQ(pushResp->m_header.m_type, FrameType::PUSH);
	EXPECT_EQ(pushResp->m_header.m_status, Status::SUCCESS);
	EXPECT_EQ(inviteDecBody.m_invitationId, m_friendRelations[4].m_id);
	EXPECT_EQ(inviteDecBody.m_friend.m_id, m_users[0].m_id);
	EXPECT_EQ(inviteDecBody.m_friend.m_username, m_users[0].m_username);
	EXPECT_EQ(inviteDecBody.m_friend.m_state, UserState::ONLINE);
	EXPECT_EQ(inviteDecBody.m_status, AddRelationStatus::ACCEPTED);
}

TEST_F(ChatServiceProtocolTest, RejectFriendHandler)
{
	auto request = std::make_shared<ProtocolFrame>();
	request->m_header.m_method = MethodType::REJECT_FRIEND_INVITATION;
	HandleInvitationRequest body;
	asio::io_context io_context;
	asio::ip::tcp::socket socket1(io_context);
	auto conn = std::make_shared<Connection>(std::move(socket1));
	conn->SetState(ConnectionState::CONNECTED);
	auto session = std::make_shared<Session>(conn);
	session->SetState(SessionState::ACTIVE);
	UserLoginAndLogout userLogin1(session, "testuser", "testpass");
	asio::ip::tcp::socket socket2(io_context);
	auto frdConn = std::make_shared<Connection>(std::move(socket2));
	frdConn->SetState(ConnectionState::CONNECTED);
	auto frdSession = std::make_shared<Session>(frdConn);
	frdSession->SetState(SessionState::ACTIVE);
	UserLoginAndLogout userLogin2(frdSession, "friendtest3", "testfriendpass3");
	// Unauthorized
	body.m_userId = m_users[0].m_id + 1;
	body.m_invitation_id = m_friendRelations[4].m_id;
	request->m_payload = Serializer::Serialize<HandleInvitationRequest>(body);
	auto response = m_service.DispatchService(session, request);
	auto respBody = Serializer::DeSerialize<StatusResponse>(response->m_payload);
	EXPECT_EQ(response->m_header.m_status, Status::UNAUTHORIZED);
	EXPECT_EQ(respBody.m_error_msg, "Unauthorized access!");
	// InvitationNotFound
	body.m_userId = m_users[0].m_id;
	body.m_invitation_id = m_friendRelations[5].m_id;
	request->m_payload = Serializer::Serialize<HandleInvitationRequest>(body);
	response = m_service.DispatchService(session, request);
	respBody = Serializer::DeSerialize<StatusResponse>(response->m_payload);
	EXPECT_EQ(response->m_header.m_status, Status::BAD_REQUEST);
	EXPECT_EQ(respBody.m_error_msg, "Invitation not found!");
	// Status::SUCCESS
	body.m_invitation_id = m_friendRelations[4].m_id;
	request->m_payload = Serializer::Serialize<HandleInvitationRequest>(body);
	response = m_service.DispatchService(session, request);
	respBody = Serializer::DeSerialize<StatusResponse>(response->m_payload);
	std::string responseStr = frdConn->GetSendQueue().front();
	frdConn->GetSendQueue().pop();
	auto pushResp = ProtocolCodec().UnPackProtocolFrame(std::string(responseStr.begin(), responseStr.end()).data(), responseStr.size())[0];
	auto inviteDecBody = Serializer::DeSerialize<FriendInvitationDecision>(pushResp->m_payload);
	EXPECT_EQ(response->m_header.m_status, Status::SUCCESS);
	EXPECT_TRUE(respBody.m_error_msg.empty());
	EXPECT_EQ(pushResp->m_header.m_method, MethodType::PUSH_FRIEND_INVITATION_DECISION);
	EXPECT_EQ(pushResp->m_header.m_type, FrameType::PUSH);
	EXPECT_EQ(pushResp->m_header.m_status, Status::SUCCESS);
	EXPECT_EQ(inviteDecBody.m_invitationId, m_friendRelations[4].m_id);
	EXPECT_EQ(inviteDecBody.m_friend.m_id, m_users[0].m_id);
	EXPECT_EQ(inviteDecBody.m_friend.m_username, m_users[0].m_username);
	EXPECT_EQ(inviteDecBody.m_friend.m_state, UserState::ONLINE);
	EXPECT_EQ(inviteDecBody.m_status, AddRelationStatus::REJECTED);
}