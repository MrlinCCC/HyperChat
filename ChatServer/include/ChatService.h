#pragma once
#include <Protocol.h>
#include <functional>
#include <Connection.h>
#include "UncopybleAndUnmovable.h"
#include <Serializer.hpp>
#include "Dto.hpp"
#include "SQLExecuser.h"
#include "Globals.hpp"

class ChatService : public UncopybleAndUnmovable
{
public:
	using ServiceHandler = std::function<ProtocolResponse::Ptr(const Connection::Ptr &, const ProtocolRequest::Ptr &)>;

	static ChatService &GetInstance();

	inline const std::shared_ptr<SQLExecuser> &GetSQLExecuser() const
	{
		return p_sqlExecuser;
	}

	template <typename Handler, typename RequestType, typename ResponseType>
	void RegisterServiceHandler(std::string method, Handler &&handler)
	{
		static_assert(std::is_invocable_r_v<ResponseType, Handler, const Connection::Ptr &, const RequestType &, Status &>,
					  "Handler must be callable with (Connection::Ptr,RequestType,Status) and return ResponseType");
		if (m_serviceHandlerMap.find(method) != m_serviceHandlerMap.end())
		{
			LOG_WARN("Handler for this MethodType already exists!");
			return;
		}
		auto serviceHandler = std::function<ProtocolResponse::Ptr(const Connection::Ptr &, const ProtocolRequest::Ptr &)>(
			[handler = std::forward<Handler>(handler)](const Connection::Ptr &conn, const ProtocolRequest::Ptr &message)
			{
				RequestType req;
				ProtocolResponse::Ptr protocolResponse = std::make_shared<ProtocolResponse>();
				try
				{
					req = Serializer::DeSerialize<RequestType>(message->m_payload);
				}
				catch (const std::exception &e)
				{
					protocolResponse->m_status = BAD_REQUEST;
					protocolResponse->m_requestId = message->m_requestId;
					LOG_WARN("DeSerialize error:{}", e.what());
					return protocolResponse;
				}
				Status status;
				ResponseType response = handler(conn, req, status);
				protocolResponse->m_status = status;
				protocolResponse->m_requestId = message->m_requestId;
				protocolResponse->m_payload = Serializer::Serialize(response);
				return protocolResponse;
			});
		m_serviceHandlerMap.insert({method, serviceHandler});
	}
	ProtocolResponse::Ptr DispatchService(const std::shared_ptr<Connection> &conn, const ProtocolRequest::Ptr &ProtocolRequest);

private:
	ChatService();

	void InitDatabase();

	bool CheckAuth(uint32_t userId, const Connection::Ptr &conn);

	bool IsUserOnline(uint32_t userId);

	const Connection::Ptr &ChatService::GetUserConnection(uint32_t userId);

	template <typename T>
	void PushOnlineData(const Connection::Ptr &conn, const T &data, const std::string &pushType)
	{
		auto response = std::make_shared<ProtocolResponse>();
		response->m_requestId = 0;
		response->m_pushType = pushType;
		response->m_status = SUCCESS;
		response->m_payload = Serializer::Serialize(data);
		conn->AsyncWriteMessage(ProtocolCodec::Instance().PackProtocolResponse(response));
	}

	StatusResponse RegisterHandler(const Connection::Ptr &conn, const AuthRequest &request, Status &status);

	AuthResponse LoginHandler(const Connection::Ptr &conn, const AuthRequest &request, Status &status);

	StatusResponse LogoutHandler(const Connection::Ptr &conn, const UserIdRequest &request, Status &status);

	ChatRoomResponse CreateChatRoomHandler(const Connection::Ptr &conn, const CreateChatRoomRequest &request, Status &status);

	ChatRoomInvitationResponse InviteToChatRoomHandler(const Connection::Ptr &conn, const InviteToChatRoomRequest &request, Status &status);

	ChatRoomResponse AcceptChatRoomtInvitationHandler(const Connection::Ptr &conn, const HandleInvitationRequest &request, Status &status);

	StatusResponse RejectChatRoomInvitationHandler(const Connection::Ptr &conn, const HandleInvitationRequest &request, Status &status);

	MessageResponse OneChatHandler(const Connection::Ptr &conn, const OneChatRequest &request, Status &status);

	MessageResponse GroupChatHandler(const Connection::Ptr &conn, const GroupChatRequest &request, Status &status);

	FriendInvitationResponse AddFriendHandler(const Connection::Ptr &conn, const AddFriendRequest &request, Status &status);

	UserResponse AcceptFriendHandler(const Connection::Ptr &conn, const HandleInvitationRequest &request, Status &status);

	StatusResponse RejectFriendHandler(const Connection::Ptr &conn, const HandleInvitationRequest &request, Status &status);

	std::unordered_map<std::string, ServiceHandler> m_serviceHandlerMap;

	std::shared_ptr<SQLExecuser> p_sqlExecuser;

	std::unordered_map<uint32_t, User> m_userMap;
	std::unordered_map<uint32_t, Connection::Ptr> m_userConnMap;

	std::mutex m_userMtx;
	std::mutex m_userConnMtx;

	friend class ChatServer;
};

constexpr const char *QueryUserById = "SELECT * FROM users WHERE id = ?";
constexpr const char *QueryUserByUsername = "SELECT * FROM users WHERE username = ?";
constexpr const char *InsertUser = "INSERT INTO users (username, password) VALUES (?, ?)";
constexpr const char *QueryFriendsByUserId = R"(
    SELECT u.id,u.username FROM friends f JOIN users u ON f.friend_id = u.id WHERE f.user_id = ? AND f.status = 1
)";
constexpr const char *QueryFriendsInvitationByFrdId = R"(
    SELECT f.id AS id,u.id AS user_id,u.username As username FROM friends f JOIN users u ON f.friend_id = u.id WHERE f.friend_id = ? AND f.status = 0
)";
constexpr const char *QueryFriendRelationsByUserIdAndFrdId = R"(
    SELECT f.id, f.user_id, f.friend_id, f.status FROM friends f WHERE (f.user_id = ? AND f.friend_id = ?) OR (f.user_id = ? AND f.friend_id = ?)
)";
constexpr const char *QueryFriendRelationByIdAndUId = "SELECT f.id, f.user_id, f.friend_id, f.status FROM friends f WHERE f.id = ? AND f.friend_id = ? AND f.status=0";
constexpr const char *InsertFriend = "INSERT INTO friends (user_id, friend_id, status) VALUES (?, ?, ?)";
constexpr const char *UpdateFriendStatus = "UPDATE friends SET status = ? WHERE id = ?";
constexpr const char *InsertMessage = "INSERT INTO messages (room_id, sender_id, message) VALUES (?, ?, ?)";
constexpr const char *QueryOfflineMsgsByUserId = R"(
    SELECT m.id,m.room_id,m.sender_id,m.message,m.created_at FROM offline_recipients r JOIN messages m ON r.message_id = m.id WHERE recipient_id = ?
)";
constexpr const char *InsertOfflineRecipient = "INSERT INTO offline_recipients (message_id, recipient_id) VALUES (?, ?)";
constexpr const char *DeleteOfflineMsgsByUserId = "DELETE FROM offline_recipients WHERE recipient_id = ?";
constexpr const char *QueryChatRoomByCrId = "SELECT cr.id,cr.name,cr.owner_id FROM chat_rooms cr WHERE cr.id=?";
constexpr const char *InsertChatRoom = "INSERT INTO chat_rooms (name,owner_id) VALUES (?, ?)";
constexpr const char *QueryChatRoomByUserId = R"(
    SELECT cr.id,cr.name,cr.owner_id FROM chat_room_members m JOIN chat_rooms cr ON m.room_id = cr.id WHERE m.user_id = ? AND m.status=1
)";
constexpr const char *QueryChatRoomMemberByCrId = "SELECT m.id, m.room_id, m.user_id, m.role FROM chat_room_members m WHERE m.room_id = ? AND m.status=1";
constexpr const char *QueryChatRoomMemberByCrIdAndUId = "SELECT m.id, m.room_id, m.user_id, m.role FROM chat_room_members m WHERE m.room_id = ? AND m.user_id = ? AND m.status=1";
constexpr const char *QueryChatRoomInvitationByIdAndUId = R"(
	SELECT m.id AS member_id,m.room_id AS room_id,m.inviter_id AS inviter_id,cr.name AS room_name,cr.owner_id AS owner_id, u.username AS username 
	FROM chat_room_members m JOIN users u ON m.inviter_id = u.id JOIN chat_rooms cr ON m.room_id = cr.id WHERE m.id = ? AND m.user_id = ? AND m.status = 0
)";
constexpr const char *QueryChatRoomInvitationByUId = R"(
	SELECT m.id AS member_id,m.room_id AS room_id,m.inviter_id AS inviter_id,cr.name AS room_name,cr.owner_id AS owner_id, u.username AS username 
	FROM chat_room_members m JOIN users u ON m.inviter_id = u.id JOIN chat_rooms cr ON m.room_id = cr.id WHERE m.user_id = ? AND m.status = 0
)";
constexpr const char *InsertChatRoomMember = "INSERT INTO chat_room_members (room_id,user_id,role,status,inviter_id) VALUES (?, ?, ?, ?, ?)";
constexpr const char *UpdateChatRoomMemberStatus = "UPDATE chat_room_members SET status = ? WHERE id = ?";
