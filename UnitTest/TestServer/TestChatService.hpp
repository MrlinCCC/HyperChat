#pragma once
#include "gtest/gtest.h"
#include "ChatService.h"
#include "Utils.hpp"
#include "SQLiteCpp/SQLiteCpp.h"

class ChatServiceProtocolTest : public ::testing::Test
{
protected:
	ChatService& m_service = ChatService::GetInstance();
	const std::shared_ptr<SQLExecuser>& m_execuser = m_service.GetSQLExecuser();
	std::shared_ptr<SQLite::Database> m_txConn;

	void SetUp() override
	{
		m_txConn = m_execuser->BeginTransaction();
		InsertTest();
	}

	void TearDown() override
	{
		m_execuser->RollbackTransaction();
	}

	void InsertTest()
	{
		std::vector<std::string> sqls = {
			"INSERT INTO users (username, password) VALUES ('testuser', '" + Sha256("testpass") + "')",
			"INSERT INTO users (username, password) VALUES ('friendtest', '" + Sha256("testfriendpass") + "')",
			"INSERT INTO users (username, password) VALUES ('friendtest1', '" + Sha256("testfriendpass1") + "')",
			"INSERT INTO users (username, password) VALUES ('friendtest2', '" + Sha256("testfriendpass2") + "')",

			"INSERT INTO friends (user_id, friend_id, status) "
			"SELECT u1.id, u2.id, 1 FROM users u1, users u2 WHERE u1.username = 'testuser' AND u2.username = 'friendtest'",
			"INSERT INTO friends (user_id, friend_id, status) "
			"SELECT u1.id, u2.id, 1 FROM users u1, users u2 WHERE u1.username = 'testuser' AND u2.username = 'friendtest1'",

			"INSERT INTO chat_rooms (name, owner_id) "
			"SELECT 'Test Room', u.id FROM users u WHERE u.username = 'testuser'",
			"INSERT INTO chat_rooms (name, owner_id) "
			"SELECT 'Test Room1', u.id FROM users u WHERE u.username = 'friendtest2'",

			"INSERT INTO chat_room_members (room_id, user_id, role) "
			"SELECT r.id, u.id, 2 FROM chat_rooms r, users u WHERE r.name = 'Test Room' AND u.username = 'testuser'",
			"INSERT INTO chat_room_members (room_id, user_id, role) "
			"SELECT r.id, u.id, 1 FROM chat_rooms r, users u WHERE r.name = 'Test Room' AND u.username = 'friendtest'",
			"INSERT INTO chat_room_members (room_id, user_id, role) "
			"SELECT r.id, u.id, 0 FROM chat_rooms r, users u WHERE r.name = 'Test Room' AND u.username = 'friendtest1'",
			"INSERT INTO chat_room_members (room_id, user_id, role) "
			"SELECT r.id, u.id, 2 FROM chat_rooms r, users u WHERE r.name = 'Test Room1' AND u.username = 'friendtest2'",
			"INSERT INTO chat_room_members (room_id, user_id, role) "
			"SELECT r.id, u.id, 1 FROM chat_rooms r, users u WHERE r.name = 'Test Room1' AND u.username = 'friendtest'",

			"INSERT INTO messages (room_id, sender_id, message) "
			"SELECT NULL, u2.id, 'Test private message' FROM users u2 WHERE u2.username = 'friendtest'",
			"INSERT INTO messages (room_id, sender_id, message) "
			"SELECT NULL, u2.id, 'Test private message1' FROM users u2 WHERE u2.username = 'testuser'",
			"INSERT INTO messages (room_id, sender_id, message) "
			"SELECT r.id, u.id, 'Test public message' FROM chat_rooms r, users u WHERE r.name = 'Test Room' AND u.username = 'friendtest'",
			"INSERT INTO messages (room_id, sender_id, message) "
			"SELECT r.id, u.id, 'Test public message1' FROM chat_rooms r, users u WHERE r.name = 'Test Room' AND u.username = 'friendtest1'",

			"INSERT INTO offline_recipients (message_id, recipient_id) "
			"SELECT m.id, u1.id FROM messages m, users u1 WHERE m.message = 'Test private message' AND u1.username = 'testuser'",
			"INSERT INTO offline_recipients (message_id, recipient_id) "
			"SELECT m.id, u1.id FROM messages m, users u1 WHERE m.message = 'Test private message1' AND u1.username = 'friendtest1'",
			"INSERT INTO offline_recipients (message_id, recipient_id) "
			"SELECT m.id, u1.id FROM messages m, users u1 WHERE m.message = 'Test public message1' AND u1.username = 'testuser'",
		};

		for (const auto& sql : sqls)
		{
			m_txConn->exec(sql);
		}
	}
};

TEST_F(ChatServiceProtocolTest, RegisterHandler)
{

	auto request = std::make_shared<ProtocolRequest>();
	request->m_method = "REGISTER";
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

TEST_F(ChatServiceProtocolTest, LoginHandler)
{
	auto request = std::make_shared<ProtocolRequest>();
	request->m_method = "LOGIN";
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

	// UserAlreadyOnline
	response = m_service.DispatchService(conn, request);
	respBody = Serializer::DeSerialize<AuthResponse>(response->m_payload);
	EXPECT_EQ(response->m_status, BAD_REQUEST);
	EXPECT_EQ(respBody.m_error_msg, "User has online!");
}