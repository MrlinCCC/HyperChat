#include "ChatService.h"
#include "Utils.hpp"

ChatService::ChatService()
{
	InitDatabase();
	using RegisterHandlerT = std::function<StatusResponse(const Session::Ptr &, const AuthRequest &, Status &)>;
	auto registerHandler = std::bind(&ChatService::RegisterHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	RegisterServiceHandler<RegisterHandlerT, AuthRequest, StatusResponse>(MethodType::REGISTER, registerHandler);

	using LoginHandlerT = std::function<AuthResponse(const Session::Ptr &, const AuthRequest &, Status &)>;
	auto loginHandler = std::bind(&ChatService::LoginHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	RegisterServiceHandler<LoginHandlerT, AuthRequest, AuthResponse>(MethodType::LOGIN, loginHandler);

	using LoginOutHandlerT = std::function<StatusResponse(const Session::Ptr &, const UserIdRequest &, Status &)>;
	auto loginOutHandler = std::bind(&ChatService::LogoutHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	RegisterServiceHandler<LoginOutHandlerT, UserIdRequest, StatusResponse>(MethodType::LOGOUT, loginOutHandler);

	using CreateChatRoomHandlerT = std::function<ChatRoomResponse(const Session::Ptr &, const CreateChatRoomRequest &, Status &)>;
	auto createChatRoomHandler = std::bind(&ChatService::CreateChatRoomHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	RegisterServiceHandler<CreateChatRoomHandlerT, CreateChatRoomRequest, ChatRoomResponse>(MethodType::CREATE_CHAT_ROOM, createChatRoomHandler);

	using InviteToChatRoomT = std::function<ChatRoomInvitationResponse(const Session::Ptr &, const InviteToChatRoomRequest &, Status &)>;
	auto inviteToChatRoomHandler = std::bind(&ChatService::InviteToChatRoomHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	RegisterServiceHandler<InviteToChatRoomT, InviteToChatRoomRequest, ChatRoomInvitationResponse>(MethodType::INVITE_TO_CHAT_ROOM, inviteToChatRoomHandler);

	using AcceptChatRoomInvitationT = std::function<ChatRoomResponse(const Session::Ptr &, const HandleInvitationRequest &, Status &)>;
	auto acceptChatRoomInvitationHandler = std::bind(&ChatService::AcceptChatRoomInvitationHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	RegisterServiceHandler<AcceptChatRoomInvitationT, HandleInvitationRequest, ChatRoomResponse>(MethodType::ACCEPT_CHAT_ROOM_INVITATION, acceptChatRoomInvitationHandler);

	using RejectChatRoomInvitationT = std::function<StatusResponse(const Session::Ptr &, const HandleInvitationRequest &, Status &)>;
	auto rejectChatRoomInvitationHandler = std::bind(&ChatService::RejectChatRoomInvitationHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	RegisterServiceHandler<RejectChatRoomInvitationT, HandleInvitationRequest, StatusResponse>(MethodType::REJECT_CHAT_ROOM_INVITATION, rejectChatRoomInvitationHandler);

	using OneChatHandlerT = std::function<MessageResponse(const Session::Ptr &, const OneChatRequest &, Status &)>;
	auto oneChatHandler = std::bind(&ChatService::OneChatHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	RegisterServiceHandler<OneChatHandlerT, OneChatRequest, MessageResponse>(MethodType::ONE_CHAT, oneChatHandler);

	using GroupChatHandlerT = std::function<MessageResponse(const Session::Ptr &, const GroupChatRequest &, Status &)>;
	auto groupChatHandler = std::bind(&ChatService::GroupChatHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	RegisterServiceHandler<GroupChatHandlerT, GroupChatRequest, MessageResponse>(MethodType::GROUP_CHAT, groupChatHandler);

	using AddFriendHandlerT = std::function<FriendInvitationResponse(const Session::Ptr &, const AddFriendRequest &, Status &)>;
	auto addFriendHandler = std::bind(&ChatService::AddFriendHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	RegisterServiceHandler<AddFriendHandlerT, AddFriendRequest, FriendInvitationResponse>(MethodType::ADD_FRIEND, addFriendHandler);

	using AcceptFriendInvitationT = std::function<UserResponse(const Session::Ptr &, const HandleInvitationRequest &, Status &)>;
	auto acceptFriendInvitationHandler = std::bind(&ChatService::AcceptFriendHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	RegisterServiceHandler<AcceptFriendInvitationT, HandleInvitationRequest, UserResponse>(MethodType::ACCEPT_FRIEND_INVITATION, acceptFriendInvitationHandler);

	using RejectFriendInvitationT = std::function<StatusResponse(const Session::Ptr &, const HandleInvitationRequest &, Status &)>;
	auto rejectFriendInvitationHandler = std::bind(&ChatService::RejectFriendHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	RegisterServiceHandler<RejectFriendInvitationT, HandleInvitationRequest, StatusResponse>(MethodType::REJECT_FRIEND_INVITATION, rejectFriendInvitationHandler);
}

void ChatService::RemoveUserSession(uint32_t userId)
{
	std::lock_guard<std::mutex> lock(m_userIdSessionMtx);
	m_userIdSessionMap.erase(userId);
}

void ChatService::InitDatabase()
{
	auto sqlPool = std::make_shared<SQLiteConnectionPool>(DB_PATH, SQL_CONNECTION_NUMS);
	auto execuser = std::make_shared<SQLExecuser>(sqlPool);
	p_sqlExecuser = execuser;
	// CreateTable
	constexpr char *createUserTable = R"(
            CREATE TABLE IF NOT EXISTS users (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                username TEXT NOT NULL UNIQUE,
                password TEXT NOT NULL
            );
        )";
	// 朋友关系表和邀请表合并
	constexpr char *createFriendTable = R"(
            CREATE TABLE IF NOT EXISTS friends (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                user_id INTEGER NOT NULL,
                friend_id INTEGER NOT NULL,
                status INTEGER NOT NULL DEFAULT 0,
                UNIQUE(user_id, friend_id)
            );
        )";

	constexpr char *createMessageTable = R"(
            CREATE TABLE IF NOT EXISTS messages (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                room_id INTEGER,
                sender_id INTEGER NOT NULL,
                message TEXT NOT NULL,
                created_at INTEGER NOT NULL DEFAULT (strftime('%s', 'now')),
				FOREIGN KEY(room_id) REFERENCES chat_rooms(id),
				FOREIGN KEY(sender_id) REFERENCES users(id)
            );
        )";

	constexpr char *createOffLineRecpTable = R"(
            CREATE TABLE IF NOT EXISTS offline_recipients (
                message_id INTEGER NOT NULL,
                recipient_id TEXT NOT NULL,
                PRIMARY KEY (message_id, recipient_id),
                FOREIGN KEY (message_id) REFERENCES messages(id) ON DELETE CASCADE
            );
        )";

	constexpr char *createChatRoomTable = R"(
            CREATE TABLE IF NOT EXISTS chat_rooms (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                name TEXT NOT NULL,
                owner_id INTEGER NOT NULL
            );
        )";
	// 聊天室成员关系表和邀请表合并
	constexpr char *createChatRoomMembersTable = R"(
            CREATE TABLE IF NOT EXISTS chat_room_members (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                room_id INTEGER NOT NULL,
                user_id INTEGER NOT NULL,
                role INTEGER NOT NULL DEFAULT 0,
				status INTEGER NOT NULL DEFAULT 0,
				inviter_id INTEGER,
                UNIQUE(room_id, user_id),
                FOREIGN KEY(room_id) REFERENCES chat_rooms(id)
            );
        )";

	const std::vector<std::string> indexSqls = {
		"CREATE INDEX IF NOT EXISTS idx_user_status ON friends(user_id, status);",
		"CREATE INDEX IF NOT EXISTS idx_friend_status ON friends(friend_id, status);",
		"CREATE INDEX IF NOT EXISTS idx_room_receiver ON messages(room_id, sender_id);",
		"CREATE INDEX IF NOT EXISTS idx_offline_recipient ON offline_recipients(recipient_id);",
		"CREATE INDEX IF NOT EXISTS idx_chat_rooms_owner ON chat_rooms(owner_id);",
		"CREATE INDEX IF NOT EXISTS idx_room_status ON chat_room_members(room_id, status);",
		"CREATE INDEX IF NOT EXISTS idx_user_status ON chat_room_members(user_id, status);"};
	std::vector<std::string> createTableSqls = {
		createUserTable,
		createFriendTable,
		createOffLineRecpTable,
		createMessageTable,
		createChatRoomTable,
		createChatRoomMembersTable};
	createTableSqls.insert(createTableSqls.end(), indexSqls.begin(), indexSqls.end());
	try
	{
		auto session = p_sqlExecuser->BeginTransaction();
		for (const std::string &sql : createTableSqls)
			session->exec(sql);
		p_sqlExecuser->CommitTransaction();
	}
	catch (const std::exception &e)
	{
		p_sqlExecuser->RollbackTransaction();
		LOG_ERROR("Occur error when creating Table", e.what());
	}
	// more
	LOG_INFO("InitDatabase Success!");
}

bool ChatService::CheckAuth(uint32_t userId, const Session::Ptr &session)
{
	return userId == session->GetUser().m_id;
}

bool ChatService::IsUserOnline(uint32_t userId)
{
	std::lock_guard<std::mutex> lock(m_userIdSessionMtx);
	return m_userIdSessionMap.find(userId) != m_userIdSessionMap.end();
}

const Session::Ptr &ChatService::GetUserSession(uint32_t userId)
{
	std::lock_guard<std::mutex> lock(m_userIdSessionMtx);
	auto it = m_userIdSessionMap.find(userId);
	if (it != m_userIdSessionMap.end())
	{
		return it->second;
	}
	static Session::Ptr nullConnPtr = nullptr;
	return nullConnPtr;
}

ProtocolFrame::Ptr ChatService::DispatchService(const std::shared_ptr<Session> &session, const ProtocolFrame::Ptr &ProtocolFrame)
{
	MethodType method = ProtocolFrame->m_header.m_method;
	if (m_serviceHandlerMap.find(method) == m_serviceHandlerMap.end())
	{
		LOG_WARN("Handler for this method not found!");
		// todo router to method no found service
		return nullptr;
	}
	// todo verify token
	return m_serviceHandlerMap[method](session, ProtocolFrame);
}

StatusResponse ChatService::RegisterHandler(const Session::Ptr &session, const AuthRequest &request, Status &status)
{
	StatusResponse response;
	if (request.m_username.empty() || request.m_passwd.empty())
	{
		status = Status::BAD_REQUEST;
		response.m_error_msg = "Username or passwd is empty!";
		return response;
	}
	auto results = p_sqlExecuser->ExecuteQuery<User>(QueryUserByUsername, request.m_username);
	if (results.size() >= 1)
	{
		status = Status::BAD_REQUEST;
		response.m_error_msg = "Username has been registered!";
		return response;
	}
	p_sqlExecuser->ExecuteUpdate(InsertUser, request.m_username, Sha256(request.m_passwd));
	status = Status::SUCCESS;
	return response;
}

AuthResponse ChatService::LoginHandler(const Session::Ptr &session, const AuthRequest &request, Status &status)
{
	AuthResponse response;
	if (request.m_username.empty() || request.m_passwd.empty())
	{
		status = Status::BAD_REQUEST;
		response.m_error_msg = "Username or passwd is empty!";
		return response;
	}
	auto results = p_sqlExecuser->ExecuteQuery<User>(QueryUserByUsername, request.m_username);

	if (results.size() < 1)
	{
		status = Status::BAD_REQUEST;
		response.m_error_msg = "Username has not registered!";
		return response;
	}
	User user = results[0];
	{
		if (IsUserOnline(user.m_id))
		{
			status = Status::BAD_REQUEST;
			response.m_error_msg = "User has online!";
			return response;
		}
	}
	if (user.m_passwd != Sha256(request.m_passwd))
	{
		status = Status::BAD_REQUEST;
		response.m_error_msg = "Passwd error!";
		return response;
	}
	{
		std::lock_guard<std::mutex> lock(m_userIdSessionMtx);
		m_userIdSessionMap[user.m_id] = session;
	}
	{
		std::lock_guard<std::mutex> lock(m_userIdSessionMtx);
		user.m_state = UserState::ONLINE;
		session->BindUser(user);
		session->SetAuthState(AuthState::AUTHENTICATED);
		m_userIdSessionMap[user.m_id] = session;
	}
	response.m_user = user;
	response.m_chatRooms = p_sqlExecuser->ExecuteQuery<ChatRoom>(QueryChatRoomByUserId, user.m_id);
	response.m_friends = p_sqlExecuser->ExecuteQuery<User>(QueryFriendsByUserId, user.m_id);
	for (auto &frd : response.m_friends)
	{
		{
			frd.m_state = IsUserOnline(frd.m_id) ? m_userIdSessionMap[frd.m_id]->GetUser().m_state : UserState::OFFLINE;
		}
	}
	try
	{
		p_sqlExecuser->BeginTransaction();
		response.m_offlineMessages = p_sqlExecuser->ExecuteQuery<Message>(QueryOfflineMsgsByUserId, user.m_id);
		p_sqlExecuser->ExecuteUpdate(DeleteOfflineMsgsByUserId, user.m_id); // todo client confirmation
		p_sqlExecuser->CommitTransaction();
	}
	catch (const std::exception &e)
	{
		p_sqlExecuser->RollbackTransaction();
		status = Status::INTERNAL_ERROR;
		response.m_error_msg = "Server internal error!";
		LOG_ERROR("LoginHandler exception: {}", e.what());
		return response;
	}
	response.m_friendInvitations = p_sqlExecuser->ExecuteQuery<FriendInvitation>(QueryFriendsInvitationByFrdId, user.m_id);
	response.m_chatRoomInvitations = p_sqlExecuser->ExecuteQuery<ChatRoomInvitation>(QueryChatRoomInvitationByUId, user.m_id);
	status = Status::SUCCESS;
	return response;
}

StatusResponse ChatService::LogoutHandler(const Session::Ptr &session, const UserIdRequest &request, Status &status)
{
	StatusResponse response;
	if (!CheckAuth(request.m_userId, session))
	{
		status = Status::UNAUTHORIZED;
		response.m_error_msg = "Unauthorized access!";
		return response;
	}
	{
		std::lock_guard<std::mutex> lock(m_userIdSessionMtx);
		m_userIdSessionMap.erase(request.m_userId);
	}
	status = Status::SUCCESS;
	session->SetAuthState(AuthState::UnAUTHENTICATED);
	session->UnbindUser();
	return response;
}

ChatRoomResponse ChatService::CreateChatRoomHandler(const Session::Ptr &session, const CreateChatRoomRequest &request, Status &status)
{
	ChatRoomResponse response;
	if (!CheckAuth(request.m_ownerId, session))
	{
		status = Status::UNAUTHORIZED;
		response.m_error_msg = "Unauthorized access!";
		return response;
	}
	if (request.m_name.empty())
	{
		status = Status::BAD_REQUEST;
		response.m_error_msg = "Chat room name is empty!";
		return response;
	}
	p_sqlExecuser->BeginTransaction();
	try
	{
		response.m_chatRoom = p_sqlExecuser->ExecuteUpdateAndReturn<ChatRoom>(InsertChatRoom, request.m_name, request.m_ownerId);
		p_sqlExecuser->ExecuteUpdate(InsertChatRoomMember, response.m_chatRoom.m_id, request.m_ownerId, ChatRoomRole::OWNER, AddRelationStatus::ACCEPTED, nullptr);
		p_sqlExecuser->CommitTransaction();
	}
	catch (const std::exception &e)
	{
		p_sqlExecuser->RollbackTransaction();
		status = Status::INTERNAL_ERROR;
		response.m_error_msg = "Server internal error!";
		LOG_ERROR("CreateChatRoomHandler exception: {}", e.what());
	}
	status = Status::SUCCESS;
	return response;
}

ChatRoomInvitationResponse ChatService::InviteToChatRoomHandler(const Session::Ptr &session, const InviteToChatRoomRequest &request, Status &status)
{
	ChatRoomInvitationResponse response;
	if (!CheckAuth(request.m_inviterId, session))
	{
		status = Status::UNAUTHORIZED;
		response.m_error_msg = "Unauthorized access!";
		return response;
	}
	auto results = p_sqlExecuser->ExecuteQuery<ChatRoomMember>(QueryChatRoomMemberByCrIdAndUId, request.m_roomId, request.m_inviteeId);
	if (results.size() > 0)
	{
		status = Status::BAD_REQUEST;
		response.m_error_msg = "User has already in chat room!";
		return response;
	}
	p_sqlExecuser->BeginTransaction();
	try
	{
		auto member = p_sqlExecuser->ExecuteUpdateAndReturn<ChatRoomMember>(InsertChatRoomMember, request.m_roomId, request.m_inviteeId, ChatRoomRole::MEMBER, AddRelationStatus::PENDING, request.m_inviterId);
		response.m_invitation.m_id = member.m_id;
		{
			std::lock_guard<std::mutex> lock(m_userIdSessionMtx);
			response.m_invitation.m_inviter = m_userIdSessionMap[request.m_inviterId]->GetUser();
		}
		response.m_invitation.m_chatRoom = p_sqlExecuser->ExecuteQuery<ChatRoom>(QueryChatRoomByCrId, member.m_roomId)[0];
		if (auto inviteeConn = GetUserSession(request.m_inviteeId))
		{
			PushOnlineData<ChatRoomInvitation>(inviteeConn, response.m_invitation, MethodType::PUSH_CHAT_ROOM_INVITATION);
		}
		p_sqlExecuser->CommitTransaction();
	}
	catch (const std::exception &e)
	{
		p_sqlExecuser->RollbackTransaction();
		status = Status::INTERNAL_ERROR;
		response.m_error_msg = "Server internal error!";
		LOG_ERROR("InvateToChatRoomHandler exception: {}", e.what());
		return response;
	}
	status = Status::SUCCESS;
	return response;
}

ChatRoomResponse ChatService::AcceptChatRoomInvitationHandler(const Session::Ptr &session, const HandleInvitationRequest &request, Status &status)
{
	ChatRoomResponse response;
	if (!CheckAuth(request.m_userId, session))
	{
		status = Status::UNAUTHORIZED;
		response.m_error_msg = "Unauthorized access!";
		return response;
	}
	auto results = p_sqlExecuser->ExecuteQuery<ChatRoomInvitation>(QueryChatRoomInvitationByIdAndUId, request.m_invitation_id, request.m_userId);
	if (results.size() < 1)
	{
		status = Status::BAD_REQUEST;
		response.m_error_msg = "Invitation not found!";
		return response;
	}
	auto invitation = results[0];
	p_sqlExecuser->ExecuteUpdate(UpdateChatRoomMemberStatus, AddRelationStatus::ACCEPTED, request.m_invitation_id);
	response.m_chatRoom = p_sqlExecuser->ExecuteQuery<ChatRoom>(QueryChatRoomByCrId, invitation.m_chatRoom.m_id)[0];
	if (auto userSess = GetUserSession(invitation.m_inviter.m_id))
	{
		ChatRoomInvitationDecision decision;
		decision.m_invitationId = invitation.m_id;
		decision.m_chatRoom = invitation.m_chatRoom;
		decision.m_status = AddRelationStatus::ACCEPTED;
		PushOnlineData<ChatRoomInvitationDecision>(userSess, decision, MethodType::PUSH_CHAT_ROOM_INVITATION_DECISION);
	}
	status = Status::SUCCESS;
	return response;
}

StatusResponse ChatService::RejectChatRoomInvitationHandler(const Session::Ptr &session, const HandleInvitationRequest &request, Status &status)
{
	StatusResponse response;
	if (!CheckAuth(request.m_userId, session))
	{
		status = Status::UNAUTHORIZED;
		response.m_error_msg = "Unauthorized access!";
		return response;
	}
	auto results = p_sqlExecuser->ExecuteQuery<ChatRoomInvitation>(QueryChatRoomInvitationByIdAndUId, request.m_invitation_id, request.m_userId);
	if (results.size() < 1)
	{
		status = Status::BAD_REQUEST;
		response.m_error_msg = "Invitation not found!";
		return response;
	}
	auto invitation = results[0];
	p_sqlExecuser->ExecuteUpdate(UpdateChatRoomMemberStatus, AddRelationStatus::REJECTED, request.m_invitation_id);
	if (auto userSess = GetUserSession(invitation.m_inviter.m_id))
	{
		ChatRoomInvitationDecision decision;
		decision.m_invitationId = invitation.m_id;
		decision.m_chatRoom = invitation.m_chatRoom;
		decision.m_status = AddRelationStatus::REJECTED;
		PushOnlineData<ChatRoomInvitationDecision>(userSess, decision, MethodType::PUSH_CHAT_ROOM_INVITATION_DECISION);
	}
	status = Status::SUCCESS;
	return response;
}

MessageResponse ChatService::OneChatHandler(const Session::Ptr &session, const OneChatRequest &request, Status &status)
{
	MessageResponse response;
	if (!CheckAuth(request.m_senderId, session))
	{
		status = Status::UNAUTHORIZED;
		response.m_error_msg = "Unauthorized access!";
		return response;
	}
	if (request.m_receiverId == request.m_senderId)
	{
		status = Status::BAD_REQUEST;
		response.m_error_msg = "Cannot send message to self!";
		return response;
	}
	if (request.m_message.empty())
	{
		status = Status::BAD_REQUEST;
		response.m_error_msg = "Message is empty!";
		return response;
	}
	p_sqlExecuser->BeginTransaction();
	try
	{
		response.m_message = p_sqlExecuser->ExecuteUpdateAndReturn<Message>(InsertMessage, nullptr, request.m_senderId, request.m_message);
		if (auto receiverConn = GetUserSession(request.m_receiverId))
		{
			PushOnlineData<Message>(receiverConn, response.m_message, MethodType::PUSH_MESSAGE);
		}
		else
		{
			p_sqlExecuser->ExecuteUpdate(InsertOfflineRecipient, response.m_message.m_id, request.m_receiverId);
		}
		p_sqlExecuser->CommitTransaction();
	}
	catch (const std::exception &e)
	{
		p_sqlExecuser->RollbackTransaction();
		status = Status::INTERNAL_ERROR;
		response.m_error_msg = "Server internal error!";
		LOG_ERROR("OneChatHandler exception: {}", e.what());
		return response;
	}
	status = Status::SUCCESS;
	return response;
}

MessageResponse ChatService::GroupChatHandler(const Session::Ptr &session, const GroupChatRequest &request, Status &status)
{
	MessageResponse response;
	if (!CheckAuth(request.m_senderId, session))
	{
		status = Status::UNAUTHORIZED;
		response.m_error_msg = "Unauthorized access!";
		return response;
	}
	if (request.m_message.empty())
	{
		status = Status::BAD_REQUEST;
		response.m_error_msg = "Message is empty!";
		return response;
	}
	p_sqlExecuser->BeginTransaction();
	try
	{
		response.m_message = p_sqlExecuser->ExecuteUpdateAndReturn<Message>(InsertMessage, request.m_roomId, request.m_senderId, request.m_message);
		bool isUserInChatRoom = false;
		auto members = p_sqlExecuser->ExecuteQuery<ChatRoomMember>(QueryChatRoomMemberByCrId, request.m_roomId);
		for (const auto &member : members)
		{
			if (member.m_userId == request.m_senderId)
			{
				isUserInChatRoom = true;
				continue;
			}
			if (auto receiverConn = GetUserSession(member.m_userId))
			{
				PushOnlineData<Message>(receiverConn, response.m_message, MethodType::PUSH_MESSAGE);
			}
			else
			{
				p_sqlExecuser->ExecuteUpdate(InsertOfflineRecipient, response.m_message.m_id, member.m_userId);
			}
		}
		if (isUserInChatRoom)
		{
			p_sqlExecuser->CommitTransaction();
		}
		else
		{
			p_sqlExecuser->RollbackTransaction();
			status = Status::FORBIDDEN;
			response.m_error_msg = "User not in group!";
			return response;
		}
	}
	catch (const std::exception &e)
	{
		p_sqlExecuser->RollbackTransaction();
		status = Status::INTERNAL_ERROR;
		response.m_error_msg = "Server internal error!";
		LOG_ERROR("GroupChatHandler exception: {}", e.what());
		return response;
	}
	status = Status::SUCCESS;
	return response;
}

FriendInvitationResponse ChatService::AddFriendHandler(const Session::Ptr &session, const AddFriendRequest &request, Status &status)
{
	FriendInvitationResponse response;
	if (!CheckAuth(request.m_userId, session))
	{
		status = Status::UNAUTHORIZED;
		response.m_error_msg = "Unauthorized access!";
		return response;
	}
	if (request.m_friendId == request.m_userId)
	{
		status = Status::BAD_REQUEST;
		response.m_error_msg = "Cannot add self as friend!";
		return response;
	}
	auto results = p_sqlExecuser->ExecuteQuery<FriendRelation>(
		QueryFriendRelationsByUserIdAndFrdId, request.m_userId, request.m_friendId, request.m_friendId, request.m_userId);
	if (results.size() > 0 && results[0].m_status == AddRelationStatus::ACCEPTED)
	{
		status = Status::BAD_REQUEST;
		response.m_error_msg = "Friend already exists!";
		return response;
	}
	if (results.size() > 0 && results[0].m_status == AddRelationStatus::PENDING)
	{
		status = Status::BAD_REQUEST;
		response.m_error_msg = "Friend request is pending!";
		return response;
	}
	auto friendRlt = p_sqlExecuser->ExecuteUpdateAndReturn<FriendRelation>(InsertFriend, request.m_userId, request.m_friendId, AddRelationStatus::PENDING);
	response.m_invitation.m_id = friendRlt.m_id;
	{
		std::lock_guard<std::mutex> lock(m_userIdSessionMtx);
		response.m_invitation.m_inviter = m_userIdSessionMap[request.m_userId]->GetUser();
	}
	if (auto friendConn = GetUserSession(request.m_friendId))
	{
		PushOnlineData<FriendInvitation>(friendConn, response.m_invitation, MethodType::PUSH_FRIEND_INVITATION);
	}
	status = Status::SUCCESS;
	return response;
}

UserResponse ChatService::AcceptFriendHandler(const Session::Ptr &session, const HandleInvitationRequest &request, Status &status)
{
	UserResponse response;
	if (!CheckAuth(request.m_userId, session))
	{
		status = Status::UNAUTHORIZED;
		response.m_error_msg = "Unauthorized access!";
		return response;
	}
	auto results = p_sqlExecuser->ExecuteQuery<FriendRelation>(QueryFriendRelationByIdAndUId, request.m_invitation_id, request.m_userId);
	if (results.size() < 1)
	{
		status = Status::BAD_REQUEST;
		response.m_error_msg = "Invitation not found!";
		return response;
	}
	auto invitation = results[0];
	p_sqlExecuser->ExecuteUpdate(UpdateFriendStatus, AddRelationStatus::ACCEPTED, request.m_invitation_id);
	p_sqlExecuser->ExecuteUpdate(InsertFriend, invitation.m_friendId, invitation.m_userId, AddRelationStatus::ACCEPTED); // 双向关系
	if (auto userSess = GetUserSession(invitation.m_userId))
	{
		{
			std::lock_guard<std::mutex> lock(m_userIdSessionMtx);
			response.m_user = m_userIdSessionMap[invitation.m_userId]->GetUser();
		}
		FriendInvitationDecision decision;
		decision.m_invitationId = request.m_invitation_id;
		{
			std::lock_guard<std::mutex> lock(m_userIdSessionMtx);
			decision.m_friend = m_userIdSessionMap[request.m_userId]->GetUser();
		}
		decision.m_status = AddRelationStatus::ACCEPTED;
		PushOnlineData<FriendInvitationDecision>(userSess, decision, MethodType::PUSH_FRIEND_INVITATION_DECISION);
	}
	else
	{
		response.m_user = p_sqlExecuser->ExecuteQuery<User>(QueryUserById, invitation.m_userId)[0];
	}
	status = Status::SUCCESS;
	return response;
}

StatusResponse ChatService::RejectFriendHandler(const Session::Ptr &session, const HandleInvitationRequest &request, Status &status)
{
	StatusResponse response;
	if (!CheckAuth(request.m_userId, session))
	{
		status = Status::UNAUTHORIZED;
		response.m_error_msg = "Unauthorized access!";
		return response;
	}
	auto results = p_sqlExecuser->ExecuteQuery<FriendRelation>(QueryFriendRelationByIdAndUId, request.m_invitation_id, request.m_userId);
	if (results.size() < 1)
	{
		status = Status::BAD_REQUEST;
		response.m_error_msg = "Invitation not found!";
		return response;
	}
	auto invitation = results[0];
	p_sqlExecuser->ExecuteUpdate(UpdateFriendStatus, AddRelationStatus::REJECTED, request.m_invitation_id);
	if (auto userSess = GetUserSession(invitation.m_userId))
	{
		FriendInvitationDecision decision;
		decision.m_invitationId = request.m_invitation_id;
		{
			std::lock_guard<std::mutex> lock(m_userIdSessionMtx);
			decision.m_friend = m_userIdSessionMap[request.m_userId]->GetUser();
		}
		decision.m_status = AddRelationStatus::REJECTED;
		PushOnlineData<FriendInvitationDecision>(userSess, decision, MethodType::PUSH_FRIEND_INVITATION_DECISION);
	}
	status = Status::SUCCESS;
	return response;
}