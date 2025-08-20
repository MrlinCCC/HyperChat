#pragma once
#include <iostream>
#include <tuple>
#include "Utils.hpp"

template<typename T>
inline T ToUint(const std::string& str, T defaultValue = static_cast<T>(0)) noexcept
{
	static_assert(std::is_integral<T>::value || std::is_enum<T>::value,
		"ToUint only supports integral or enum types");
	if (str.empty())
		return defaultValue;

	try
	{
		if constexpr (std::is_enum<T>::value)
		{
			using Underlying = typename std::underlying_type<T>::type;
			Underlying value = static_cast<Underlying>(std::stoull(str));
			return static_cast<T>(value);
		}
		else
		{
			return static_cast<T>(std::stoull(str));
		}
	}
	catch (...)
	{
		return defaultValue;
	}
}

enum UserState : uint8_t
{
	ONLINE = 0,
	OFFLINE = 1,
	STATE_NULL = 255
};

enum AddRelationStatus : uint8_t
{
	PENDING = 0,
	ACCEPTED = 1,
	REJECTED = 2
};

enum ChatRoomRole : uint8_t
{
	MEMBER = 0,
	ADMIN = 1,
	OWNER = 2,
	ROLE_NULL = 255
};

struct User
{
	uint32_t m_id;
	std::string m_username;
	std::string m_passwd;
	UserState m_state;

	User()
		: m_id(0), m_username(""), m_passwd(""), m_state(UserState::STATE_NULL) {
	}

	User(uint32_t id, const std::string& username, const std::string& password, UserState state)
		: m_id(id), m_username(username), m_passwd(password), m_state(state) {
	}

	auto Tie()& { return std::tie(m_id, m_username, m_state); }
	auto Tie() const& { return std::tie(m_id, m_username, m_state); }

	static User FromMap(const std::unordered_map<std::string, std::string>& row)
	{
		return User{
			ToUint<uint32_t>(GetOrDefault(row, "id")),
			GetOrDefault(row, "username"),
			GetOrDefault(row, "password"),
			UserState::STATE_NULL };
	}
};

struct FriendRelation
{
	uint32_t m_id;
	uint32_t m_userId;
	uint32_t m_friendId;
	AddRelationStatus m_status;
	FriendRelation()
		: m_id(0), m_userId(0), m_friendId(0), m_status(AddRelationStatus::PENDING) {
	}
	FriendRelation(uint32_t id, uint32_t userId, uint32_t friendId, AddRelationStatus status)
		: m_id(id), m_userId(userId), m_friendId(friendId), m_status(status) {
	}
	auto Tie()& { return std::tie(m_id, m_userId, m_friendId, m_status); }
	auto Tie() const& { return std::tie(m_id, m_userId, m_friendId, m_status); }
	static FriendRelation FromMap(const std::unordered_map<std::string, std::string>& row)
	{
		return FriendRelation{
			ToUint<uint32_t>(GetOrDefault(row, "id")),
			ToUint<uint32_t>(GetOrDefault(row, "user_id")),
			ToUint<uint32_t>(GetOrDefault(row, "friend_id")),
			ToUint<AddRelationStatus>(GetOrDefault(row, "status")) };
	}
};

struct Message
{
	uint32_t m_id;
	uint32_t m_roomId;
	uint32_t m_senderId;
	std::string m_message;
	uint32_t m_createdAt;

	Message()
		: m_id(0), m_roomId(0), m_senderId(0), m_message(""), m_createdAt(0) {
	}

	Message(uint32_t id, uint32_t roomId, uint32_t senderId, const std::string& message, uint32_t createdAt)
		: m_id(id), m_roomId(roomId), m_senderId(senderId), m_message(message), m_createdAt(createdAt) {
	}

	auto Tie()& { return std::tie(m_id, m_roomId, m_senderId, m_message, m_createdAt); }
	auto Tie() const& { return std::tie(m_id, m_roomId, m_senderId, m_message, m_createdAt); }

	static Message FromMap(const std::unordered_map<std::string, std::string>& row)
	{
		return Message{
			ToUint<uint32_t>(GetOrDefault(row, "id")),
			ToUint<uint32_t>(GetOrDefault(row, "room_id")),
			ToUint<uint32_t>(GetOrDefault(row, "sender_id")),
			GetOrDefault(row, "message"),
			ToUint<uint32_t>(GetOrDefault(row, "created_at")) };
	}
};

struct ChatRoom
{
	uint32_t m_id;
	std::string m_name;
	uint32_t m_ownerId;

	ChatRoom()
		: m_id(0), m_name(""), m_ownerId(0) {
	}

	ChatRoom(uint32_t id, const std::string& name, uint32_t ownerId)
		: m_id(id), m_name(name), m_ownerId(ownerId) {
	}

	auto Tie()& { return std::tie(m_id, m_name, m_ownerId); }
	auto Tie() const& { return std::tie(m_id, m_name, m_ownerId); }

	static ChatRoom FromMap(const std::unordered_map<std::string, std::string>& row)
	{
		return ChatRoom{
			ToUint<uint32_t>(GetOrDefault(row, "id")),
			GetOrDefault(row, "name"),
			ToUint<uint32_t>(GetOrDefault(row, "owner_id")),
		};
	}
};

struct ChatRoomMember
{
	uint32_t m_id;
	uint32_t m_roomId;
	uint32_t m_userId;
	ChatRoomRole m_role;
	AddRelationStatus m_status;
	ChatRoomMember()
		: m_id(0), m_roomId(0), m_userId(0), m_role(ChatRoomRole::ROLE_NULL), m_status(AddRelationStatus::PENDING) {
	}
	ChatRoomMember(uint32_t id, uint32_t roomId, uint32_t userId, ChatRoomRole role, AddRelationStatus status)
		: m_id(id), m_roomId(roomId), m_userId(userId), m_role(role), m_status(status) {
	}
	auto Tie()& { return std::tie(m_id, m_roomId, m_userId, m_role, m_status); }
	auto Tie() const& { return std::tie(m_id, m_roomId, m_userId, m_role, m_status); }
	static ChatRoomMember FromMap(const std::unordered_map<std::string, std::string>& row)
	{
		return ChatRoomMember{
			ToUint<uint32_t>(GetOrDefault(row, "id")),
			ToUint<uint32_t>(GetOrDefault(row, "room_id")),
			ToUint<uint32_t>(GetOrDefault(row, "user_id")),
			ToUint<ChatRoomRole>(GetOrDefault(row, "role")),
			ToUint<AddRelationStatus>(GetOrDefault(row, "status"))
		};
	}
};

struct FriendInvitation {
	uint32_t m_id;
	User m_inviter;
	FriendInvitation() = default;
	FriendInvitation(uint32_t id, const User& inviter)
		: m_id(id), m_inviter(inviter) {
	}
	auto Tie()& { return std::tie(m_id, m_inviter); }
	auto Tie() const& { return std::tie(m_id, m_inviter); }
	static FriendInvitation FromMap(const std::unordered_map<std::string, std::string>& row)
	{
		return FriendInvitation{
			ToUint<uint32_t>(GetOrDefault(row, "id")),
			{ToUint<uint32_t>(GetOrDefault(row, "user_id")),
				GetOrDefault(row, "username"),
				"",
				UserState::STATE_NULL}
		};
	}
};

struct ChatRoomInvitation {
	uint32_t m_id;
	ChatRoom m_chatRoom;
	User m_inviter;
	ChatRoomInvitation() = default;
	ChatRoomInvitation(uint32_t id, const ChatRoom& chatRoom, const User& inviter)
		: m_id(id), m_chatRoom(chatRoom), m_inviter(inviter) {
	}
	auto Tie()& { return std::tie(m_id, m_chatRoom, m_inviter); }
	auto Tie() const& { return std::tie(m_id, m_chatRoom, m_inviter); }
	static ChatRoomInvitation FromMap(const std::unordered_map<std::string, std::string>& row)
	{
		return ChatRoomInvitation{
			ToUint<uint32_t>(GetOrDefault(row, "member_id")),
			{ToUint<uint32_t>(GetOrDefault(row, "room_id")),
				GetOrDefault(row, "room_name"),
				ToUint<uint32_t>(GetOrDefault(row, "owner_id"))},
			{ToUint<uint32_t>(GetOrDefault(row, "inviter_id")),
				GetOrDefault(row, "username"),
				"",
				UserState::STATE_NULL}
		};
	}
};

struct FriendInvitationDecision {
	uint32_t m_invitationId;
	User m_friend;
	AddRelationStatus m_status;
	FriendInvitationDecision() = default;
	FriendInvitationDecision(uint32_t invitationId, User frd, AddRelationStatus status)
		: m_invitationId(invitationId), m_friend(frd), m_status(status) {
	}
	auto Tie()& { return std::tie(m_invitationId, m_friend, m_status); }
	auto Tie() const& { return std::tie(m_invitationId, m_friend, m_status); }
};

struct ChatRoomInvitationDecision {
	uint32_t m_invitationId;
	ChatRoom m_chatRoom;
	AddRelationStatus m_status;
	ChatRoomInvitationDecision() = default;
	ChatRoomInvitationDecision(uint32_t invitationId, ChatRoom chatRoom, AddRelationStatus status)
		: m_invitationId(invitationId), m_chatRoom(chatRoom), m_status(status) {
	}
	auto Tie()& { return std::tie(m_invitationId, m_chatRoom, m_status); }
	auto Tie() const& { return std::tie(m_invitationId, m_chatRoom, m_status); }
};

struct UserIdRequest {
	uint32_t m_userId;
	UserIdRequest() = default;
	UserIdRequest(uint32_t userId)
		: m_userId(userId) {
	}
	auto Tie()& { return std::tie(m_userId); }
	auto Tie() const& { return std::tie(m_userId); }
};

struct StatusResponse
{
	std::string m_error_msg;

	StatusResponse() = default;
	StatusResponse(const std::string& error_msg)
		: m_error_msg(error_msg) {
	}

	auto Tie()& { return std::tie(m_error_msg); }
	auto Tie() const& { return std::tie(m_error_msg); }
};

struct AuthRequest
{
	std::string m_username;
	std::string m_passwd;

	AuthRequest() = default;
	AuthRequest(const std::string& username, const std::string& passwd)
		: m_username(username), m_passwd(passwd) {
	}

	auto Tie()& { return std::tie(m_username, m_passwd); }
	auto Tie() const& { return std::tie(m_username, m_passwd); }
};

struct AuthResponse
{
	User m_user;
	std::vector<Message> m_offlineMessages;
	std::vector<User> m_friends;
	std::vector<FriendInvitation> m_friendInvitations;
	std::vector<ChatRoom> m_chatRooms;
	std::vector<ChatRoomInvitation> m_chatRoomInvitations;
	std::string m_error_msg;

	AuthResponse() = default;

	auto Tie()& { return std::tie(m_user, m_offlineMessages, m_friends, m_friendInvitations, m_chatRooms, m_chatRoomInvitations, m_error_msg); }
	auto Tie() const& { return std::tie(m_user, m_offlineMessages, m_friends, m_friendInvitations, m_chatRooms, m_chatRoomInvitations, m_error_msg); }
};

struct UserResponse {
	User m_user;
	std::string m_error_msg;
	UserResponse() = default;
	auto Tie()& { return std::tie(m_user, m_error_msg); }
	auto Tie() const& { return std::tie(m_user, m_error_msg); }
};

struct MessageResponse {
	Message m_message;
	std::string m_error_msg;
	MessageResponse() = default;
	MessageResponse(const Message& message, const std::string& error_msg = "")
		: m_message(message), m_error_msg(error_msg) {
	}
	auto Tie()& { return std::tie(m_message, m_error_msg); }
	auto Tie() const& { return std::tie(m_message, m_error_msg); }
};

struct CreateChatRoomRequest
{
	std::string m_name;
	uint32_t m_ownerId;
	CreateChatRoomRequest() = default;
	CreateChatRoomRequest(const std::string& name, uint32_t ownerId)
		: m_name(name), m_ownerId(ownerId) {
	}
	auto Tie()& { return std::tie(m_name, m_ownerId); }
	auto Tie() const& { return std::tie(m_name, m_ownerId); }
};

struct ChatRoomResponse
{
	ChatRoom m_chatRoom;
	std::string m_error_msg;
	ChatRoomResponse() = default;

	auto Tie()& { return std::tie(m_chatRoom, m_error_msg); }
	auto Tie() const& { return std::tie(m_chatRoom, m_error_msg); }
};

struct InviteToChatRoomRequest {
	uint32_t m_roomId;
	uint32_t m_inviterId;
	uint32_t m_inviteeId;
	InviteToChatRoomRequest() = default;
	InviteToChatRoomRequest(uint32_t roomId, uint32_t inviterId, uint32_t inviteeId)
		: m_roomId(roomId), m_inviterId(inviterId), m_inviteeId(inviteeId) {
	}
	auto Tie()& { return std::tie(m_roomId, m_inviterId, m_inviteeId); }
	auto Tie() const& { return std::tie(m_roomId, m_inviterId, m_inviteeId); }
};

struct ChatRoomInvitationResponse {
	ChatRoomInvitation m_invitation;
	std::string m_error_msg;
	ChatRoomInvitationResponse() = default;
	auto Tie()& { return std::tie(m_invitation, m_error_msg); }
	auto Tie() const& { return std::tie(m_invitation, m_error_msg); }
};

struct OneChatRequest
{
	uint32_t m_senderId;
	uint32_t m_receiverId;
	std::string m_message;
	OneChatRequest() = default;
	OneChatRequest(uint32_t senderId, uint32_t receiverId, const std::string& message)
		: m_senderId(senderId), m_receiverId(receiverId), m_message(message) {
	}
	auto Tie()& { return std::tie(m_senderId, m_receiverId, m_message); }
	auto Tie() const& { return std::tie(m_senderId, m_receiverId, m_message); }
};

struct GroupChatRequest {
	uint32_t m_senderId;
	uint32_t m_roomId;
	std::string m_message;
	GroupChatRequest() = default;
	GroupChatRequest(uint32_t senderId, uint32_t roomId, const std::string& message)
		: m_senderId(senderId), m_roomId(roomId), m_message(message) {
	}
	auto Tie()& { return std::tie(m_senderId, m_roomId, m_message); }
	auto Tie() const& { return std::tie(m_senderId, m_roomId, m_message); }
};

struct AddFriendRequest {
	uint32_t m_userId;
	uint32_t m_friendId;
	AddFriendRequest() = default;
	AddFriendRequest(uint32_t userId, uint32_t friendId)
		: m_userId(userId), m_friendId(friendId) {
	}
	auto Tie()& { return std::tie(m_userId, m_friendId); }
	auto Tie() const& { return std::tie(m_userId, m_friendId); }
};

struct FriendInvitationResponse {
	FriendInvitation m_invitation;
	std::string m_error_msg;
	FriendInvitationResponse() = default;
	auto Tie()& { return std::tie(m_invitation, m_error_msg); }
	auto Tie() const& { return std::tie(m_invitation, m_error_msg); }
};

struct HandleInvitationRequest {
	uint32_t m_invitation_id;
	uint32_t m_userId;

	HandleInvitationRequest() = default;
	HandleInvitationRequest(uint32_t invitation_id, uint32_t userId)
		: m_invitation_id(invitation_id), m_userId(userId) {
	}
	auto Tie()& { return std::tie(m_invitation_id, m_userId); }
	auto Tie() const& { return std::tie(m_invitation_id, m_userId); }
};

namespace MethodType {
	constexpr const char* Register = "Register";
	constexpr const char* Login = "Login";
	constexpr const char* Logout = "Logout";
	constexpr const char* CreateChatRoom = "CreateChatRoom";
	constexpr const char* InviteToChatRoom = "InviteToChatRoom";
	constexpr const char* AcceptChatRoomInvitation = "AcceptChatRoomInvitation";
	constexpr const char* RejectChatRoomInvitation = "RejectChatRoomInvitation";
	constexpr const char* OneChat = "OneChat";
	constexpr const char* GroupChat = "GroupChat";
	constexpr const char* AddFriend = "AddFriend";
	constexpr const char* AcceptFriendInvitation = "AcceptFriendInvitation";
	constexpr const char* RejectFriendInvitation = "RejectFriendInvitation";
}

namespace PushType {
	constexpr const char* Message = "Message";
	constexpr const char* ChatRoomInvitation = "ChatRoomInvitation";
	constexpr const char* ChatRoomInvitationDecision = "ChatRoomInvitationDecision";
	constexpr const char* FriendInvitation = "FriendInvitation";
	constexpr const char* FriendInvitationDesicion = "FriendInvitationDesicion";
}