#pragma once
#include "gtest/gtest.h"
#include "ChatService.h"
#include "Utils.hpp"
#include "SQLiteCpp/SQLiteCpp.h"

class ChatServiceProtocolTest : public ::testing::Test
{
protected:
	ChatService &m_service = ChatService::GetInstance();
	const std::shared_ptr<SQLExecuser> &m_execuser = m_service.GetSQLExecuser();
	std::vector<User> m_users;
	std::vector<FriendRelation> m_friendRelations;
	std::vector<ChatRoom> m_chatrooms;
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
		m_chatrooms.push_back(m_execuser->ExecuteUpdateAndReturn<ChatRoom>(
			"INSERT INTO chat_rooms (name, owner_id) VALUES ('Test Room', " + std::to_string(m_users[0].m_id) + ")"));
		m_chatrooms.push_back(m_execuser->ExecuteUpdateAndReturn<ChatRoom>(
			"INSERT INTO chat_rooms (name, owner_id) VALUES ('Test Room', " + std::to_string(m_users[2].m_id) + ")"));
		m_chatrooms.push_back(m_execuser->ExecuteUpdateAndReturn<ChatRoom>(
			"INSERT INTO chat_rooms (name, owner_id) VALUES ('Test Room1', " + std::to_string(m_users[3].m_id) + ")"));
		m_chatroomMembers.push_back(m_execuser->ExecuteUpdateAndReturn<ChatRoomMember>(
			"INSERT INTO chat_room_members (room_id, user_id, status, role) VALUES (" + std::to_string(m_chatrooms[0].m_id) + ", " + std::to_string(m_users[0].m_id) + ", 1, 2)"));
		m_chatroomMembers.push_back(m_execuser->ExecuteUpdateAndReturn<ChatRoomMember>(
			"INSERT INTO chat_room_members (room_id, user_id, status, role) VALUES (" + std::to_string(m_chatrooms[0].m_id) + ", " + std::to_string(m_users[1].m_id) + ", 1, 2)"));
		m_chatroomMembers.push_back(m_execuser->ExecuteUpdateAndReturn<ChatRoomMember>(
			"INSERT INTO chat_room_members (room_id, user_id, status, role) VALUES (" + std::to_string(m_chatrooms[0].m_id) + ", " + std::to_string(m_users[2].m_id) + ", 1, 0)"));
		m_chatroomMembers.push_back(m_execuser->ExecuteUpdateAndReturn<ChatRoomMember>(
			"INSERT INTO chat_room_members (room_id, user_id, status, role) VALUES (" + std::to_string(m_chatrooms[1].m_id) + ", " + std::to_string(m_users[3].m_id) + ", 1, 2)"));
		m_chatroomMembers.push_back(m_execuser->ExecuteUpdateAndReturn<ChatRoomMember>(
			"INSERT INTO chat_room_members (room_id, user_id, status, role) VALUES (" + std::to_string(m_chatrooms[1].m_id) + ", " + std::to_string(m_users[1].m_id) + ", 1, 1)"));
		m_chatroomMembers.push_back(m_execuser->ExecuteUpdateAndReturn<ChatRoomMember>(
			"INSERT INTO chat_room_members (room_id, user_id, status, role) VALUES (" + std::to_string(m_chatrooms[2].m_id) + ", " + std::to_string(m_users[3].m_id) + ", 1, 1)"));
		m_chatroomMembers.push_back(m_execuser->ExecuteUpdateAndReturn<ChatRoomMember>(
			"INSERT INTO chat_room_members (room_id, user_id, inviter_id, status, role) VALUES (" + std::to_string(m_chatrooms[2].m_id) + ", " + std::to_string(m_users[0].m_id) + ", " + std::to_string(m_users[3].m_id) + ", 0, 1)"));
		m_messages.push_back(m_execuser->ExecuteUpdateAndReturn<Message>(
			"INSERT INTO messages (room_id, sender_id, message) VALUES (NULL, " + std::to_string(m_users[0].m_id) + ", " + " 'Test private message')"));
		m_messages.push_back(m_execuser->ExecuteUpdateAndReturn<Message>(
			"INSERT INTO messages (room_id, sender_id, message) VALUES (NULL, " + std::to_string(m_users[1].m_id) + ", " + " 'Test private message1')"));
		m_messages.push_back(m_execuser->ExecuteUpdateAndReturn<Message>(
			"INSERT INTO messages (room_id, sender_id, message) VALUES (" + std::to_string(m_chatrooms[0].m_id) + ", " + std::to_string(m_users[1].m_id) + ", " + " 'Test public message')"));
		m_messages.push_back(m_execuser->ExecuteUpdateAndReturn<Message>(
			"INSERT INTO messages (room_id, sender_id, message) VALUES (" + std::to_string(m_chatrooms[1].m_id) + ", " + std::to_string(m_users[2].m_id) + ", " + " 'Test public message')"));
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
	auto request = std::make_shared<ProtocolRequest>();
	request->m_method = MethodType::Register;
	AuthRequest body;
	asio::io_context io_context;
	asio::ip::tcp::socket socket(io_context);
	auto conn = std::make_shared<Connection>(std::move(socket));

	// EmptyUsernameOrPassword
	body.m_username = "";
	body.m_passwd = "any";
	request->m_payload = Serializer::Serialize<AuthRequest>(body);
	auto response = m_service.DispatchService(conn, request);
	auto respBody = Serializer::DeSerialize<StatusResponse>(response->m_payload);
	EXPECT_EQ(response->m_status, BAD_REQUEST);
	EXPECT_EQ(respBody.m_error_msg, "Username or passwd is empty!");
	body.m_username = "newuser";
	body.m_passwd = "";
	request->m_payload = Serializer::Serialize<AuthRequest>(body);
	response = m_service.DispatchService(conn, request);
	respBody = Serializer::DeSerialize<StatusResponse>(response->m_payload);
	EXPECT_EQ(response->m_status, BAD_REQUEST);
	EXPECT_EQ(respBody.m_error_msg, "Username or passwd is empty!");

	// UsernameExists
	body.m_username = "testuser";
	body.m_passwd = "somepass";
	request->m_payload = Serializer::Serialize<AuthRequest>(body);
	response = m_service.DispatchService(conn, request);
	respBody = Serializer::DeSerialize<StatusResponse>(response->m_payload);
	EXPECT_EQ(response->m_status, BAD_REQUEST);
	EXPECT_EQ(respBody.m_error_msg, "Username has been registered!");

	// Success
	body.m_username = "newuser";
	body.m_passwd = "testpass";
	request->m_payload = Serializer::Serialize<AuthRequest>(body);
	response = m_service.DispatchService(conn, request);
	respBody = Serializer::DeSerialize<StatusResponse>(response->m_payload);
	EXPECT_EQ(response->m_status, SUCCESS);
	EXPECT_TRUE(respBody.m_error_msg.empty());
	auto results = m_execuser->ExecuteQuery<User>("SELECT * from users WHERE users.username = ?", body.m_username);
	EXPECT_EQ(results.size(), 1);
}

class UserLoginAndLogout
{
public:
	UserLoginAndLogout(const Connection::Ptr &conn, const std::string &username, const std::string &password)
		: m_conn(conn)
	{
		m_user = UserLogin(m_conn, username, password);
	}

	~UserLoginAndLogout()
	{
		UserLogout(m_conn, m_user.m_id);
	}

	static User UserLogin(const Connection::Ptr &conn, const std::string &username, const std::string &password)
	{
		auto request = std::make_shared<ProtocolRequest>();
		AuthRequest login_body;
		request->m_method = MethodType::Login;
		login_body.m_username = username;
		login_body.m_passwd = password;
		request->m_payload = Serializer::Serialize<AuthRequest>(login_body);
		auto response = ChatService::GetInstance().DispatchService(conn, request);
		EXPECT_EQ(response->m_status, SUCCESS);
		auto respBody = Serializer::DeSerialize<AuthResponse>(response->m_payload);
		return respBody.m_user;
	}

	static void UserLogout(const Connection::Ptr &conn, uint32_t user_id)
	{
		auto request = std::make_shared<ProtocolRequest>();
		UserIdRequest login_body;
		login_body.m_userId = user_id;
		request->m_method = MethodType::Logout;
		request->m_payload = Serializer::Serialize<UserIdRequest>(login_body);
		auto response = ChatService::GetInstance().DispatchService(conn, request);
	}

private:
	Connection::Ptr m_conn;
	User m_user;
};

TEST_F(ChatServiceProtocolTest, LoginHandler)
{
	auto request = std::make_shared<ProtocolRequest>();
	request->m_method = MethodType::Login;
	AuthRequest body;
	asio::io_context io_context;
	asio::ip::tcp::socket socket(io_context);
	auto conn = std::make_shared<Connection>(std::move(socket));

	// EmptyUsernameOrPassword
	body.m_username = "";
	body.m_passwd = "any";
	request->m_payload = Serializer::Serialize<AuthRequest>(body);
	auto response = m_service.DispatchService(conn, request);
	auto respBody = Serializer::DeSerialize<AuthResponse>(response->m_payload);
	EXPECT_EQ(response->m_status, BAD_REQUEST);
	EXPECT_EQ(respBody.m_error_msg, "Username or passwd is empty!");
	body.m_username = "newuser";
	body.m_passwd = "";
	request->m_payload = Serializer::Serialize<AuthRequest>(body);
	response = m_service.DispatchService(conn, request);
	respBody = Serializer::DeSerialize<AuthResponse>(response->m_payload);
	EXPECT_EQ(response->m_status, BAD_REQUEST);
	EXPECT_EQ(respBody.m_error_msg, "Username or passwd is empty!");

	// UserNotRegistered
	body.m_username = "nonexistent";
	body.m_passwd = "any";
	request->m_payload = Serializer::Serialize<AuthRequest>(body);
	response = m_service.DispatchService(conn, request);
	respBody = Serializer::DeSerialize<AuthResponse>(response->m_payload);
	EXPECT_EQ(response->m_status, BAD_REQUEST);
	EXPECT_EQ(respBody.m_error_msg, "Username has not registered!");

	// PasswordError
	body.m_username = "testuser";
	body.m_passwd = "wrongpasshash";
	request->m_payload = Serializer::Serialize<AuthRequest>(body);
	response = m_service.DispatchService(conn, request);
	respBody = Serializer::DeSerialize<AuthResponse>(response->m_payload);
	EXPECT_EQ(response->m_status, BAD_REQUEST);
	EXPECT_EQ(respBody.m_error_msg, "Passwd error!");

	// SUCCESS
	body.m_username = "testuser";
	body.m_passwd = "testpass";
	request->m_payload = Serializer::Serialize<AuthRequest>(body);
	response = m_service.DispatchService(conn, request);
	respBody = Serializer::DeSerialize<AuthResponse>(response->m_payload);
	EXPECT_EQ(response->m_status, SUCCESS);
	EXPECT_EQ(respBody.m_friends.size(), 2);
	EXPECT_EQ(respBody.m_offlineMessages.size(), 2);
	EXPECT_EQ(respBody.m_chatRooms.size(), 1);
	EXPECT_EQ(respBody.m_chatRoomInvitations.size(), 1);
	EXPECT_EQ(respBody.m_friendInvitations.size(), 1);
	// UserAlreadyOnline
	response = m_service.DispatchService(conn, request);
	respBody = Serializer::DeSerialize<AuthResponse>(response->m_payload);
	EXPECT_EQ(response->m_status, BAD_REQUEST);
	EXPECT_EQ(respBody.m_error_msg, "User has online!");
	// reset
	UserLoginAndLogout::UserLogout(conn, m_users[0].m_id);
}

TEST_F(ChatServiceProtocolTest, LogoutHandler)
{
	auto request = std::make_shared<ProtocolRequest>();
	request->m_method = MethodType::Logout;
	UserIdRequest body;
	asio::io_context io_context;
	asio::ip::tcp::socket socket(io_context);
	auto conn = std::make_shared<Connection>(std::move(socket));
	// Unauthorized
	body.m_userId = m_users[0].m_id + 1;
	request->m_payload = Serializer::Serialize<UserIdRequest>(body);
	auto response = m_service.DispatchService(conn, request);
	auto respBody = Serializer::DeSerialize<StatusResponse>(response->m_payload);
	EXPECT_EQ(response->m_status, UNAUTHORIZED);
	EXPECT_EQ(respBody.m_error_msg, "Unauthorized access!");
	// SUCCESS
	UserLoginAndLogout userLogin(conn, "testuser", "testpass");
	body.m_userId = m_users[0].m_id;
	request->m_method = MethodType::Logout;
	request->m_payload = Serializer::Serialize<UserIdRequest>(body);
	response = m_service.DispatchService(conn, request);
	respBody = Serializer::DeSerialize<StatusResponse>(response->m_payload);
	EXPECT_EQ(response->m_status, SUCCESS);
	EXPECT_TRUE(respBody.m_error_msg.empty());
}

TEST_F(ChatServiceProtocolTest, CreateChatRoomHandler)
{
	auto request = std::make_shared<ProtocolRequest>();
	request->m_method = MethodType::CreateChatRoom;
	CreateChatRoomRequest body;
	asio::io_context io_context;
	asio::ip::tcp::socket socket(io_context);
	auto conn = std::make_shared<Connection>(std::move(socket));
	UserLoginAndLogout userLogin(conn, "testuser", "testpass");
	// Unauthorized
	body.m_ownerId = m_users[0].m_id + 1;
	body.m_name = "New Room";
	request->m_payload = Serializer::Serialize<CreateChatRoomRequest>(body);
	auto response = m_service.DispatchService(conn, request);
	auto respBody = Serializer::DeSerialize<ChatRoomResponse>(response->m_payload);
	EXPECT_EQ(response->m_status, UNAUTHORIZED);
	EXPECT_EQ(respBody.m_error_msg, "Unauthorized access!");
	// EmptyName
	body.m_ownerId = m_users[0].m_id;
	body.m_name = "";
	request->m_payload = Serializer::Serialize<CreateChatRoomRequest>(body);
	response = m_service.DispatchService(conn, request);
	respBody = Serializer::DeSerialize<ChatRoomResponse>(response->m_payload);
	EXPECT_EQ(response->m_status, BAD_REQUEST);
	EXPECT_EQ(respBody.m_error_msg, "Chat room name is empty!");
	// SUCCESS
	body.m_name = "New Room";
	request->m_payload = Serializer::Serialize<CreateChatRoomRequest>(body);
	response = m_service.DispatchService(conn, request);
	respBody = Serializer::DeSerialize<ChatRoomResponse>(response->m_payload);
	EXPECT_EQ(response->m_status, SUCCESS);
	EXPECT_TRUE(respBody.m_error_msg.empty());
	EXPECT_EQ(respBody.m_chatRoom.m_name, body.m_name);
	EXPECT_EQ(respBody.m_chatRoom.m_ownerId, body.m_ownerId);
}

TEST_F(ChatServiceProtocolTest, InviteToChatRoomHandler)
{
	auto request = std::make_shared<ProtocolRequest>();
	request->m_method = MethodType::InviteToChatRoom;
	InviteToChatRoomRequest body;
	asio::io_context io_context;
	asio::ip::tcp::socket socket1(io_context);
	auto userConn = std::make_shared<Connection>(std::move(socket1));
	UserLoginAndLogout userLogin1(userConn, "testuser", "testpass");
	asio::ip::tcp::socket socket2(io_context);
	auto frdConn = std::make_shared<Connection>(std::move(socket2));
	UserLoginAndLogout userLogin2(frdConn, "friendtest3", "testfriendpass3");
	// Unauthorized
	body.m_inviterId = m_users[0].m_id + 1;
	body.m_roomId = m_chatrooms[0].m_id;
	body.m_inviteeId = m_users[1].m_id;
	request->m_payload = Serializer::Serialize<InviteToChatRoomRequest>(body);
	auto response = m_service.DispatchService(userConn, request);
	auto respBody = Serializer::DeSerialize<ChatRoomInvitationResponse>(response->m_payload);
	EXPECT_EQ(response->m_status, UNAUTHORIZED);
	EXPECT_EQ(respBody.m_error_msg, "Unauthorized access!");
	// UserAlreadyInChatRoom
	body.m_inviterId = m_users[0].m_id;
	body.m_roomId = m_chatrooms[0].m_id;
	body.m_inviteeId = m_users[1].m_id;
	request->m_payload = Serializer::Serialize<InviteToChatRoomRequest>(body);
	response = m_service.DispatchService(userConn, request);
	respBody = Serializer::DeSerialize<ChatRoomInvitationResponse>(response->m_payload);
	EXPECT_EQ(response->m_status, BAD_REQUEST);
	EXPECT_EQ(respBody.m_error_msg, "User has already in chat room!");
	// SUCCESS
	body.m_inviteeId = m_users[4].m_id;
	request->m_payload = Serializer::Serialize<InviteToChatRoomRequest>(body);
	response = m_service.DispatchService(userConn, request);
	respBody = Serializer::DeSerialize<ChatRoomInvitationResponse>(response->m_payload);
	EXPECT_EQ(response->m_status, SUCCESS);
	EXPECT_TRUE(respBody.m_error_msg.empty());
	std::string responseStr = frdConn->GetSendQueue().front();
	frdConn->GetSendQueue().pop();
	auto pushResp = ProtocolCodec::Instance().UnPackProtocolResponse(std::vector<char>(responseStr.begin(), responseStr.end()), responseStr.size())[0];
	auto inviteBody = Serializer::DeSerialize<ChatRoomInvitation>(pushResp->m_payload);
	EXPECT_EQ(pushResp->m_pushType, PushType::ChatRoomInvitation);
	EXPECT_EQ(pushResp->m_requestId, 0);
	EXPECT_EQ(pushResp->m_status, SUCCESS);
	EXPECT_EQ(inviteBody.m_id, respBody.m_invitation.m_id);
	EXPECT_EQ(inviteBody.m_inviter.m_id, m_users[0].m_id);
	EXPECT_EQ(inviteBody.m_inviter.m_username, m_users[0].m_username);
	EXPECT_EQ(inviteBody.m_chatRoom.m_id, m_chatrooms[0].m_id);
	EXPECT_EQ(inviteBody.m_chatRoom.m_name, m_chatrooms[0].m_name);
	EXPECT_EQ(inviteBody.m_chatRoom.m_ownerId, m_users[0].m_id);
}

TEST_F(ChatServiceProtocolTest, AcceptChatRoomInvitationHandler)
{
	auto request = std::make_shared<ProtocolRequest>();
	request->m_method = MethodType::AcceptChatRoomInvitation;
	HandleInvitationRequest body;
	asio::io_context io_context;
	asio::ip::tcp::socket socket1(io_context);
	auto conn = std::make_shared<Connection>(std::move(socket1));
	UserLoginAndLogout userLogin1(conn, "testuser", "testpass");
	asio::ip::tcp::socket socket2(io_context);
	auto frdConn = std::make_shared<Connection>(std::move(socket2));
	UserLoginAndLogout userLogin2(frdConn, "friendtest2", "testfriendpass2");
	// Unauthorized
	body.m_userId = m_users[0].m_id + 1;
	body.m_invitation_id = m_chatroomMembers[6].m_id;
	request->m_payload = Serializer::Serialize<HandleInvitationRequest>(body);
	auto response = m_service.DispatchService(conn, request);
	auto respBody = Serializer::DeSerialize<ChatRoomResponse>(response->m_payload);
	EXPECT_EQ(response->m_status, UNAUTHORIZED);
	EXPECT_EQ(respBody.m_error_msg, "Unauthorized access!");
	// InvitationNotFound
	body.m_userId = m_users[0].m_id;
	body.m_invitation_id = m_chatroomMembers[0].m_id;
	request->m_payload = Serializer::Serialize<HandleInvitationRequest>(body);
	response = m_service.DispatchService(conn, request);
	respBody = Serializer::DeSerialize<ChatRoomResponse>(response->m_payload);
	EXPECT_EQ(response->m_status, BAD_REQUEST);
	EXPECT_EQ(respBody.m_error_msg, "Invitation not found!");
	// SUCCESS
	body.m_invitation_id = m_chatroomMembers[6].m_id;
	request->m_payload = Serializer::Serialize<HandleInvitationRequest>(body);
	response = m_service.DispatchService(conn, request);
	respBody = Serializer::DeSerialize<ChatRoomResponse>(response->m_payload);
	EXPECT_EQ(response->m_status, SUCCESS);
	EXPECT_TRUE(respBody.m_error_msg.empty());
	EXPECT_EQ(respBody.m_chatRoom.m_id, m_chatrooms[2].m_id);
	std::string responseStr = frdConn->GetSendQueue().front();
	frdConn->GetSendQueue().pop();
	auto pushResp = ProtocolCodec::Instance().UnPackProtocolResponse(std::vector<char>(responseStr.begin(), responseStr.end()), responseStr.size())[0];
	auto inviteDecBody = Serializer::DeSerialize<ChatRoomInvitationDecision>(pushResp->m_payload);
	EXPECT_EQ(pushResp->m_pushType, PushType::ChatRoomInvitationDecision);
	EXPECT_EQ(pushResp->m_requestId, 0);
	EXPECT_EQ(pushResp->m_status, SUCCESS);
	EXPECT_EQ(inviteDecBody.m_invitationId, m_chatroomMembers[6].m_id);
	EXPECT_EQ(inviteDecBody.m_chatRoom.m_id, m_chatrooms[2].m_id);
	EXPECT_EQ(inviteDecBody.m_chatRoom.m_name, m_chatrooms[2].m_name);
	EXPECT_EQ(inviteDecBody.m_chatRoom.m_ownerId, m_chatrooms[2].m_ownerId);
	EXPECT_EQ(inviteDecBody.m_status, AddRelationStatus::ACCEPTED);
}

TEST_F(ChatServiceProtocolTest, RejectChatRoomInvitationHandler)
{
	auto request = std::make_shared<ProtocolRequest>();
	request->m_method = MethodType::RejectChatRoomInvitation;
	HandleInvitationRequest body;
	asio::io_context io_context;
	asio::ip::tcp::socket socket1(io_context);
	auto conn = std::make_shared<Connection>(std::move(socket1));
	UserLoginAndLogout userLogin1(conn, "testuser", "testpass");
	asio::ip::tcp::socket socket2(io_context);
	auto frdConn = std::make_shared<Connection>(std::move(socket2));
	UserLoginAndLogout userLogin2(frdConn, "friendtest2", "testfriendpass2");
	// Unauthorized
	body.m_userId = m_users[0].m_id + 1;
	body.m_invitation_id = m_chatroomMembers[6].m_id;
	request->m_payload = Serializer::Serialize<HandleInvitationRequest>(body);
	auto response = m_service.DispatchService(conn, request);
	auto respBody = Serializer::DeSerialize<StatusResponse>(response->m_payload);
	EXPECT_EQ(response->m_status, UNAUTHORIZED);
	EXPECT_EQ(respBody.m_error_msg, "Unauthorized access!");
	// InvitationNotFound
	body.m_userId = m_users[0].m_id;
	body.m_invitation_id = m_chatroomMembers[0].m_id;
	request->m_payload = Serializer::Serialize<HandleInvitationRequest>(body);
	response = m_service.DispatchService(conn, request);
	respBody = Serializer::DeSerialize<StatusResponse>(response->m_payload);
	EXPECT_EQ(response->m_status, BAD_REQUEST);
	EXPECT_EQ(respBody.m_error_msg, "Invitation not found!");
	// SUCCESS
	body.m_invitation_id = m_chatroomMembers[6].m_id;
	request->m_payload = Serializer::Serialize<HandleInvitationRequest>(body);
	response = m_service.DispatchService(conn, request);
	respBody = Serializer::DeSerialize<StatusResponse>(response->m_payload);
	std::string responseStr = frdConn->GetSendQueue().front();
	frdConn->GetSendQueue().pop();
	auto pushResp = ProtocolCodec::Instance().UnPackProtocolResponse(std::vector<char>(responseStr.begin(), responseStr.end()), responseStr.size())[0];
	auto inviteDecBody = Serializer::DeSerialize<ChatRoomInvitationDecision>(pushResp->m_payload);
	EXPECT_EQ(response->m_status, SUCCESS);
	EXPECT_TRUE(respBody.m_error_msg.empty());
	EXPECT_EQ(pushResp->m_pushType, PushType::ChatRoomInvitationDecision);
	EXPECT_EQ(pushResp->m_requestId, 0);
	EXPECT_EQ(pushResp->m_status, SUCCESS);
	EXPECT_EQ(inviteDecBody.m_invitationId, m_chatroomMembers[6].m_id);
	EXPECT_EQ(inviteDecBody.m_chatRoom.m_id, m_chatrooms[2].m_id);
	EXPECT_EQ(inviteDecBody.m_chatRoom.m_name, m_chatrooms[2].m_name);
	EXPECT_EQ(inviteDecBody.m_chatRoom.m_ownerId, m_chatrooms[2].m_ownerId);
	EXPECT_EQ(inviteDecBody.m_status, AddRelationStatus::REJECTED);
}

TEST_F(ChatServiceProtocolTest, OneChatHandler)
{
	auto request = std::make_shared<ProtocolRequest>();
	request->m_method = MethodType::OneChat;
	OneChatRequest body;
	asio::io_context io_context;
	asio::ip::tcp::socket socket1(io_context);
	auto conn = std::make_shared<Connection>(std::move(socket1));
	UserLoginAndLogout userLogin1(conn, "testuser", "testpass");
	asio::ip::tcp::socket socket2(io_context);
	auto frdConn = std::make_shared<Connection>(std::move(socket2));
	UserLoginAndLogout userLogin2(frdConn, "friendtest", "testfriendpass");
	// Unauthorized
	body.m_message = "Hello, friend!";
	body.m_receiverId = m_users[1].m_id;
	body.m_senderId = m_users[0].m_id + 1;
	request->m_payload = Serializer::Serialize<OneChatRequest>(body);
	auto response = m_service.DispatchService(conn, request);
	auto respBody = Serializer::DeSerialize<MessageResponse>(response->m_payload);
	EXPECT_EQ(response->m_status, UNAUTHORIZED);
	EXPECT_EQ(respBody.m_error_msg, "Unauthorized access!");
	// Send to self
	body.m_receiverId = m_users[0].m_id;
	body.m_senderId = m_users[0].m_id;
	request->m_payload = Serializer::Serialize<OneChatRequest>(body);
	response = m_service.DispatchService(conn, request);
	respBody = Serializer::DeSerialize<MessageResponse>(response->m_payload);
	EXPECT_EQ(response->m_status, BAD_REQUEST);
	EXPECT_EQ(respBody.m_error_msg, "Cannot send message to self!");
	// Message empty
	body.m_message = "";
	body.m_receiverId = m_users[1].m_id;
	request->m_payload = Serializer::Serialize<OneChatRequest>(body);
	response = m_service.DispatchService(conn, request);
	respBody = Serializer::DeSerialize<MessageResponse>(response->m_payload);
	EXPECT_EQ(response->m_status, BAD_REQUEST);
	EXPECT_EQ(respBody.m_error_msg, "Message is empty!");
	// Success
	body.m_message = "Hello, friend!";
	request->m_payload = Serializer::Serialize<OneChatRequest>(body);
	response = m_service.DispatchService(conn, request);
	respBody = Serializer::DeSerialize<MessageResponse>(response->m_payload);
	std::string responseStr = frdConn->GetSendQueue().front();
	frdConn->GetSendQueue().pop();
	auto pushResp = ProtocolCodec::Instance().UnPackProtocolResponse(std::vector<char>(responseStr.begin(), responseStr.end()), responseStr.size())[0];
	auto messageBody = Serializer::DeSerialize<Message>(pushResp->m_payload);
	EXPECT_EQ(response->m_status, SUCCESS);
	EXPECT_TRUE(respBody.m_error_msg.empty());
	EXPECT_EQ(pushResp->m_pushType, PushType::Message);
	EXPECT_EQ(pushResp->m_requestId, 0);
	EXPECT_EQ(messageBody.m_senderId, m_users[0].m_id);
	EXPECT_EQ(messageBody.m_roomId, 0);
	EXPECT_EQ(messageBody.m_message, "Hello, friend!");
	EXPECT_EQ(messageBody.m_id, respBody.m_message.m_id);
	EXPECT_EQ(messageBody.m_createdAt, respBody.m_message.m_createdAt);
}

TEST_F(ChatServiceProtocolTest, GroupChatHandler)
{
	auto request = std::make_shared<ProtocolRequest>();
	request->m_method = MethodType::GroupChat;
	GroupChatRequest body;
	asio::io_context io_context;
	asio::ip::tcp::socket socket1(io_context);
	auto conn = std::make_shared<Connection>(std::move(socket1));
	UserLoginAndLogout userLogin1(conn, "testuser", "testpass");
	asio::ip::tcp::socket socket2(io_context);
	auto frd1Conn = std::make_shared<Connection>(std::move(socket2));
	UserLoginAndLogout userLogin2(frd1Conn, "friendtest", "testfriendpass");
	asio::ip::tcp::socket socket3(io_context);
	auto frd2Conn = std::make_shared<Connection>(std::move(socket3));
	UserLoginAndLogout userLogin3(frd2Conn, "friendtest1", "testfriendpass1");
	// Unauthorized
	body.m_message = "Hello, friends!";
	body.m_roomId = m_chatrooms[0].m_id;
	body.m_senderId = m_users[0].m_id + 1;
	request->m_payload = Serializer::Serialize<GroupChatRequest>(body);
	auto response = m_service.DispatchService(conn, request);
	auto respBody = Serializer::DeSerialize<MessageResponse>(response->m_payload);
	EXPECT_EQ(response->m_status, UNAUTHORIZED);
	EXPECT_EQ(respBody.m_error_msg, "Unauthorized access!");
	// Message empty
	body.m_message = "";
	body.m_senderId = m_users[0].m_id;
	request->m_payload = Serializer::Serialize<GroupChatRequest>(body);
	response = m_service.DispatchService(conn, request);
	respBody = Serializer::DeSerialize<MessageResponse>(response->m_payload);
	EXPECT_EQ(response->m_status, BAD_REQUEST);
	EXPECT_EQ(respBody.m_error_msg, "Message is empty!");
	// Success
	body.m_message = "Hello, friends!";
	request->m_payload = Serializer::Serialize<GroupChatRequest>(body);
	response = m_service.DispatchService(conn, request);
	respBody = Serializer::DeSerialize<MessageResponse>(response->m_payload);
	std::string responseStr = frd1Conn->GetSendQueue().front();
	frd1Conn->GetSendQueue().pop();
	auto pushResp1 = ProtocolCodec::Instance().UnPackProtocolResponse(std::vector<char>(responseStr.begin(), responseStr.end()), responseStr.size())[0];
	auto messageBody1 = Serializer::DeSerialize<Message>(pushResp1->m_payload);
	responseStr = frd2Conn->GetSendQueue().front();
	frd2Conn->GetSendQueue().pop();
	auto pushResp2 = ProtocolCodec::Instance().UnPackProtocolResponse(std::vector<char>(responseStr.begin(), responseStr.end()), responseStr.size())[0];
	auto messageBody2 = Serializer::DeSerialize<Message>(pushResp2->m_payload);
	EXPECT_EQ(response->m_status, SUCCESS);
	EXPECT_TRUE(respBody.m_error_msg.empty());
	EXPECT_EQ(pushResp1->m_pushType, PushType::Message);
	EXPECT_EQ(pushResp1->m_status, SUCCESS);
	EXPECT_EQ(pushResp1->m_requestId, 0);
	EXPECT_EQ(messageBody1.m_id, respBody.m_message.m_id);
	EXPECT_EQ(messageBody1.m_roomId, m_chatrooms[0].m_id);
	EXPECT_EQ(messageBody1.m_senderId, m_users[0].m_id);
	EXPECT_EQ(messageBody1.m_message, "Hello, friends!");
	EXPECT_EQ(pushResp2->m_pushType, PushType::Message);
	EXPECT_EQ(pushResp2->m_status, SUCCESS);
	EXPECT_EQ(pushResp2->m_requestId, 0);
	EXPECT_EQ(messageBody2.m_id, respBody.m_message.m_id);
	EXPECT_EQ(messageBody2.m_roomId, m_chatrooms[0].m_id);
	EXPECT_EQ(messageBody2.m_senderId, m_users[0].m_id);
	EXPECT_EQ(messageBody2.m_message, "Hello, friends!");
}

TEST_F(ChatServiceProtocolTest, AddFriendHandler)
{
	auto request = std::make_shared<ProtocolRequest>();
	request->m_method = MethodType::AddFriend;
	AddFriendRequest body;
	asio::io_context io_context;
	asio::ip::tcp::socket socket1(io_context);
	auto conn = std::make_shared<Connection>(std::move(socket1));
	UserLoginAndLogout userLogin1(conn, "testuser", "testpass");
	asio::ip::tcp::socket socket2(io_context);
	auto frd1Conn = std::make_shared<Connection>(std::move(socket2));
	UserLoginAndLogout userLogin2(frd1Conn, "friendtest1", "testfriendpass1");
	// Unauthorized
	body.m_userId = m_users[0].m_id + 1;
	body.m_friendId = m_users[2].m_id;
	request->m_payload = Serializer::Serialize<AddFriendRequest>(body);
	auto response = m_service.DispatchService(conn, request);
	auto respBody = Serializer::DeSerialize<FriendInvitationResponse>(response->m_payload);
	EXPECT_EQ(response->m_status, UNAUTHORIZED);
	EXPECT_EQ(respBody.m_error_msg, "Unauthorized access!");
	// Add self
	body.m_userId = m_users[0].m_id;
	body.m_friendId = m_users[0].m_id;
	request->m_payload = Serializer::Serialize<AddFriendRequest>(body);
	response = m_service.DispatchService(conn, request);
	respBody = Serializer::DeSerialize<FriendInvitationResponse>(response->m_payload);
	EXPECT_EQ(response->m_status, BAD_REQUEST);
	EXPECT_EQ(respBody.m_error_msg, "Cannot add self as friend!");
	// Already friends
	body.m_friendId = m_users[1].m_id;
	request->m_payload = Serializer::Serialize<AddFriendRequest>(body);
	response = m_service.DispatchService(conn, request);
	respBody = Serializer::DeSerialize<FriendInvitationResponse>(response->m_payload);
	EXPECT_EQ(response->m_status, BAD_REQUEST);
	EXPECT_EQ(respBody.m_error_msg, "Friend already exists!");
	// Request is pending
	body.m_friendId = m_users[4].m_id;
	request->m_payload = Serializer::Serialize<AddFriendRequest>(body);
	response = m_service.DispatchService(conn, request);
	respBody = Serializer::DeSerialize<FriendInvitationResponse>(response->m_payload);
	EXPECT_EQ(response->m_status, BAD_REQUEST);
	EXPECT_EQ(respBody.m_error_msg, "Friend request is pending!");
	// SUCCESS
	body.m_friendId = m_users[2].m_id;
	request->m_payload = Serializer::Serialize<AddFriendRequest>(body);
	response = m_service.DispatchService(conn, request);
	respBody = Serializer::DeSerialize<FriendInvitationResponse>(response->m_payload);
	EXPECT_EQ(response->m_status, SUCCESS);
	EXPECT_TRUE(respBody.m_error_msg.empty());
	std::string responseStr = frd1Conn->GetSendQueue().front();
	frd1Conn->GetSendQueue().pop();
	auto pushResp = ProtocolCodec::Instance().UnPackProtocolResponse(std::vector<char>(responseStr.begin(), responseStr.end()), responseStr.size())[0];
	auto inviteBody = Serializer::DeSerialize<FriendInvitation>(pushResp->m_payload);
	EXPECT_EQ(pushResp->m_pushType, PushType::FriendInvitation);
	EXPECT_EQ(pushResp->m_requestId, 0);
	EXPECT_EQ(pushResp->m_status, SUCCESS);
	EXPECT_EQ(inviteBody.m_id, respBody.m_invitation.m_id);
	EXPECT_EQ(inviteBody.m_inviter.m_id, m_users[0].m_id);
	EXPECT_EQ(inviteBody.m_inviter.m_username, m_users[0].m_username);
}

TEST_F(ChatServiceProtocolTest, AcceptFriendHandler)
{
	auto request = std::make_shared<ProtocolRequest>();
	request->m_method = MethodType::AcceptFriendInvitation;
	HandleInvitationRequest body;
	asio::io_context io_context;
	asio::ip::tcp::socket socket1(io_context);
	auto conn = std::make_shared<Connection>(std::move(socket1));
	UserLoginAndLogout userLogin1(conn, "testuser", "testpass");
	asio::ip::tcp::socket socket2(io_context);
	auto frdConn = std::make_shared<Connection>(std::move(socket2));
	UserLoginAndLogout userLogin2(frdConn, "friendtest3", "testfriendpass3");
	// Unauthorized
	body.m_userId = m_users[0].m_id + 1;
	body.m_invitation_id = m_friendRelations[4].m_id;
	request->m_payload = Serializer::Serialize<HandleInvitationRequest>(body);
	auto response = m_service.DispatchService(conn, request);
	auto respBody = Serializer::DeSerialize<UserResponse>(response->m_payload);
	EXPECT_EQ(response->m_status, UNAUTHORIZED);
	EXPECT_EQ(respBody.m_error_msg, "Unauthorized access!");
	// InvitationNotFound
	body.m_userId = m_users[0].m_id;
	body.m_invitation_id = m_friendRelations[5].m_id;
	request->m_payload = Serializer::Serialize<HandleInvitationRequest>(body);
	response = m_service.DispatchService(conn, request);
	respBody = Serializer::DeSerialize<UserResponse>(response->m_payload);
	EXPECT_EQ(response->m_status, BAD_REQUEST);
	EXPECT_EQ(respBody.m_error_msg, "Invitation not found!");
	// SUCCESS
	body.m_invitation_id = m_friendRelations[4].m_id;
	request->m_payload = Serializer::Serialize<HandleInvitationRequest>(body);
	response = m_service.DispatchService(conn, request);
	respBody = Serializer::DeSerialize<UserResponse>(response->m_payload);
	std::string responseStr = frdConn->GetSendQueue().front();
	frdConn->GetSendQueue().pop();
	auto pushResp = ProtocolCodec::Instance().UnPackProtocolResponse(std::vector<char>(responseStr.begin(), responseStr.end()), responseStr.size())[0];
	auto inviteDecBody = Serializer::DeSerialize<FriendInvitationDecision>(pushResp->m_payload);
	EXPECT_EQ(response->m_status, SUCCESS);
	EXPECT_TRUE(respBody.m_error_msg.empty());
	EXPECT_EQ(respBody.m_user.m_id, m_users[4].m_id);
	EXPECT_EQ(respBody.m_user.m_username, m_users[4].m_username);
	EXPECT_EQ(respBody.m_user.m_state, UserState::ONLINE);
	EXPECT_EQ(pushResp->m_pushType, PushType::FriendInvitationDesicion);
	EXPECT_EQ(pushResp->m_status, SUCCESS);
	EXPECT_EQ(pushResp->m_requestId, 0);
	EXPECT_EQ(inviteDecBody.m_invitationId, m_friendRelations[4].m_id);
	EXPECT_EQ(inviteDecBody.m_friend.m_id, m_users[0].m_id);
	EXPECT_EQ(inviteDecBody.m_friend.m_username, m_users[0].m_username);
	EXPECT_EQ(inviteDecBody.m_friend.m_state, UserState::ONLINE);
	EXPECT_EQ(inviteDecBody.m_status, AddRelationStatus::ACCEPTED);
}

TEST_F(ChatServiceProtocolTest, RejectFriendHandler)
{
	auto request = std::make_shared<ProtocolRequest>();
	request->m_method = MethodType::RejectFriendInvitation;
	HandleInvitationRequest body;
	asio::io_context io_context;
	asio::ip::tcp::socket socket1(io_context);
	auto conn = std::make_shared<Connection>(std::move(socket1));
	UserLoginAndLogout userLogin1(conn, "testuser", "testpass");
	asio::ip::tcp::socket socket2(io_context);
	auto frdConn = std::make_shared<Connection>(std::move(socket2));
	UserLoginAndLogout userLogin2(frdConn, "friendtest3", "testfriendpass3");
	// Unauthorized
	body.m_userId = m_users[0].m_id + 1;
	body.m_invitation_id = m_friendRelations[4].m_id;
	request->m_payload = Serializer::Serialize<HandleInvitationRequest>(body);
	auto response = m_service.DispatchService(conn, request);
	auto respBody = Serializer::DeSerialize<StatusResponse>(response->m_payload);
	EXPECT_EQ(response->m_status, UNAUTHORIZED);
	EXPECT_EQ(respBody.m_error_msg, "Unauthorized access!");
	// InvitationNotFound
	body.m_userId = m_users[0].m_id;
	body.m_invitation_id = m_friendRelations[5].m_id;
	request->m_payload = Serializer::Serialize<HandleInvitationRequest>(body);
	response = m_service.DispatchService(conn, request);
	respBody = Serializer::DeSerialize<StatusResponse>(response->m_payload);
	EXPECT_EQ(response->m_status, BAD_REQUEST);
	EXPECT_EQ(respBody.m_error_msg, "Invitation not found!");
	// SUCCESS
	body.m_invitation_id = m_friendRelations[4].m_id;
	request->m_payload = Serializer::Serialize<HandleInvitationRequest>(body);
	response = m_service.DispatchService(conn, request);
	respBody = Serializer::DeSerialize<StatusResponse>(response->m_payload);
	std::string responseStr = frdConn->GetSendQueue().front();
	frdConn->GetSendQueue().pop();
	auto pushResp = ProtocolCodec::Instance().UnPackProtocolResponse(std::vector<char>(responseStr.begin(), responseStr.end()), responseStr.size())[0];
	auto inviteDecBody = Serializer::DeSerialize<FriendInvitationDecision>(pushResp->m_payload);
	EXPECT_EQ(response->m_status, SUCCESS);
	EXPECT_TRUE(respBody.m_error_msg.empty());
	EXPECT_EQ(pushResp->m_pushType, PushType::FriendInvitationDesicion);
	EXPECT_EQ(pushResp->m_status, SUCCESS);
	EXPECT_EQ(pushResp->m_requestId, 0);
	EXPECT_EQ(inviteDecBody.m_invitationId, m_friendRelations[4].m_id);
	EXPECT_EQ(inviteDecBody.m_friend.m_id, m_users[0].m_id);
	EXPECT_EQ(inviteDecBody.m_friend.m_username, m_users[0].m_username);
	EXPECT_EQ(inviteDecBody.m_friend.m_state, UserState::ONLINE);
	EXPECT_EQ(inviteDecBody.m_status, AddRelationStatus::REJECTED);
}