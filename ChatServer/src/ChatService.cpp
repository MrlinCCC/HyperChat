#include "ChatService.h"
#include "Utils.hpp"

ChatService::ChatService()
{
	InitDatabase();
	using RegisterHandlerT = std::function<StatusResponse(const Connection::Ptr&, const AuthRequest&, Status&)>;
	auto registerHandler = std::bind(&ChatService::RegisterHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	RegisterServiceHandler<RegisterHandlerT, AuthRequest, StatusResponse>(MethodType::Register, registerHandler);

	using LoginHandlerT = std::function<AuthResponse(const Connection::Ptr&, const AuthRequest&, Status&)>;
	auto loginHandler = std::bind(&ChatService::LoginHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	RegisterServiceHandler<LoginHandlerT, AuthRequest, AuthResponse>(MethodType::Login, loginHandler);

	using LoginOutHandlerT = std::function<StatusResponse(const Connection::Ptr&, const UserIdRequest&, Status&)>;
	auto loginOutHandler = std::bind(&ChatService::LogoutHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	RegisterServiceHandler<LoginOutHandlerT, UserIdRequest, StatusResponse>(MethodType::Logout, loginOutHandler);

	using CreateChatRoomHandlerT = std::function<ChatRoomResponse(const Connection::Ptr&, const CreateChatRoomRequest&, Status&)>;
	auto createChatRoomHandler = std::bind(&ChatService::CreateChatRoomHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	RegisterServiceHandler<CreateChatRoomHandlerT, CreateChatRoomRequest, ChatRoomResponse>(MethodType::CreateChatRoom, createChatRoomHandler);

	using InviteToChatRoomT = std::function<ChatRoomInvitationResponse(const Connection::Ptr&, const InviteToChatRoomRequest&, Status&)>;
	auto inviteToChatRoomHandler = std::bind(&ChatService::InviteToChatRoomHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	RegisterServiceHandler<InviteToChatRoomT, InviteToChatRoomRequest, ChatRoomInvitationResponse>(MethodType::InviteToChatRoom, inviteToChatRoomHandler);

	using AcceptChatRoomtInvitationT = std::function<ChatRoomResponse(const Connection::Ptr&, const HandleInvitationRequest&, Status&)>;
	auto acceptChatRoomtInvitationHandler = std::bind(&ChatService::AcceptChatRoomtInvitationHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	RegisterServiceHandler<AcceptChatRoomtInvitationT, HandleInvitationRequest, ChatRoomResponse>(MethodType::AcceptChatRoomInvitation, acceptChatRoomtInvitationHandler);

	using RejectChatRoomtInvitationT = std::function<StatusResponse(const Connection::Ptr&, const HandleInvitationRequest&, Status&)>;
	auto rejectChatRoomtInvitationHandler = std::bind(&ChatService::RejectChatRoomInvitationHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	RegisterServiceHandler<RejectChatRoomtInvitationT, HandleInvitationRequest, StatusResponse>(MethodType::RejectChatRoomInvitation, rejectChatRoomtInvitationHandler);

	using OneChatHandlerT = std::function<MessageResponse(const Connection::Ptr&, const OneChatRequest&, Status&)>;
	auto oneChatHandler = std::bind(&ChatService::OneChatHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	RegisterServiceHandler<OneChatHandlerT, OneChatRequest, MessageResponse>(MethodType::OneChat, oneChatHandler);

	using GroupChatHandlerT = std::function<MessageResponse(const Connection::Ptr&, const GroupChatRequest&, Status&)>;
	auto groupChatHandler = std::bind(&ChatService::GroupChatHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	RegisterServiceHandler<GroupChatHandlerT, GroupChatRequest, MessageResponse>(MethodType::GroupChat, groupChatHandler);

	using AddFrinedHandlerT = std::function<FriendInvitationResponse(const Connection::Ptr&, const AddFriendRequest&, Status&)>;
	auto addFrinedHandler = std::bind(&ChatService::AddFriendHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	RegisterServiceHandler<AddFrinedHandlerT, AddFriendRequest, FriendInvitationResponse>(MethodType::AddFriend, addFrinedHandler);

	using AcceptFriendInvitationT = std::function<UserResponse(const Connection::Ptr&, const HandleInvitationRequest&, Status&)>;
	auto acceptFriendInvitationHandler = std::bind(&ChatService::AcceptFriendHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	RegisterServiceHandler<AcceptFriendInvitationT, HandleInvitationRequest, UserResponse>(MethodType::AcceptFriendInvitation, acceptFriendInvitationHandler);

	using RejectFriendInvitationT = std::function<StatusResponse(const Connection::Ptr&, const HandleInvitationRequest&, Status&)>;
	auto rejectFriendInvitationHandler = std::bind(&ChatService::RejectFriendHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	RegisterServiceHandler<RejectFriendInvitationT, HandleInvitationRequest, StatusResponse>(MethodType::RejectFriendInvitation, rejectFriendInvitationHandler);
}

ChatService& ChatService::GetInstance()
{
	static ChatService service;
	return service;
}

void ChatService::InitDatabase()
{
	auto sqlPool = std::make_shared<SQLiteConnectionPool>(DB_PATH, SQL_CONNECTION_NUMS);
	auto execuser = std::make_shared<SQLExecuser>(sqlPool);
	p_sqlExecuser = execuser;
	// CreateTable
	constexpr char* createUserTable = R"(
            CREATE TABLE IF NOT EXISTS users (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                username TEXT NOT NULL UNIQUE,
                password TEXT NOT NULL
            );
        )";
	//朋友关系表和邀请表合并
	constexpr char* createFriendTable = R"(
            CREATE TABLE IF NOT EXISTS friends (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                user_id INTEGER NOT NULL,
                friend_id INTEGER NOT NULL,
                status INTEGER NOT NULL DEFAULT 0,
                UNIQUE(user_id, friend_id)
            );
        )";


	constexpr char* createMessageTable = R"(
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

	constexpr char* createOffLineRecpTable = R"(
            CREATE TABLE IF NOT EXISTS offline_recipients (
                message_id INTEGER NOT NULL,
                recipient_id TEXT NOT NULL,
                PRIMARY KEY (message_id, recipient_id),
                FOREIGN KEY (message_id) REFERENCES messages(id) ON DELETE CASCADE
            );
        )";

	constexpr char* createChatRoomTable = R"(
            CREATE TABLE IF NOT EXISTS chat_rooms (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                name TEXT NOT NULL,
                owner_id INTEGER NOT NULL
            );
        )";
	//聊天室成员关系表和邀请表合并
	constexpr char* createChatRoomMembersTable = R"(
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
		"CREATE INDEX IF NOT EXISTS idx_user_status ON chat_room_members(user_id, status);"
	};
	std::vector<std::string> createTableSqls = {
		createUserTable,
		createFriendTable,
		createOffLineRecpTable,
		createMessageTable,
		createChatRoomTable,
		createChatRoomMembersTable
	};
	createTableSqls.insert(createTableSqls.end(), indexSqls.begin(), indexSqls.end());
	try
	{
		auto conn = p_sqlExecuser->BeginTransaction();
		for (const std::string& sql : createTableSqls)
			conn->exec(sql);
		p_sqlExecuser->CommitTransaction();
	}
	catch (const std::exception& e)
	{
		p_sqlExecuser->RollbackTransaction();
		LOG_ERROR("Occur error when creating Table", e.what());
	}
	// more
	LOG_INFO("InitDatabase Success!");
}

bool ChatService::CheckAuth(uint32_t userId, const Connection::Ptr& conn) {
	std::lock_guard<std::mutex> lock(m_userConnMtx);
	auto it = m_userConnMap.find(userId);
	if (it == m_userConnMap.end()) return false;
	return it->second == conn;
}

bool ChatService::IsUserOnline(uint32_t userId) {
	std::lock_guard<std::mutex> lock(m_userMtx);
	return m_userMap.find(userId) != m_userMap.end();
}

const Connection::Ptr& ChatService::GetUserConnection(uint32_t userId) {
	std::lock_guard<std::mutex> lock(m_userConnMtx);
	auto it = m_userConnMap.find(userId);
	if (it != m_userConnMap.end()) {
		return it->second;
	}
	static Connection::Ptr nullConnPtr = nullptr;
	return nullConnPtr;
}

ProtocolResponse::Ptr ChatService::DispatchService(const std::shared_ptr<Connection>& conn, const ProtocolRequest::Ptr& ProtocolRequest)
{
	std::string method = ProtocolRequest->m_method;
	if (m_serviceHandlerMap.find(method) == m_serviceHandlerMap.end())
	{
		LOG_WARN("Handler for this method:{} not found!", method);
		// todo router to method no found service
		return nullptr;
	}
	// todo verify token
	return m_serviceHandlerMap[method](conn, ProtocolRequest);
}

StatusResponse ChatService::RegisterHandler(const Connection::Ptr& conn, const AuthRequest& request, Status& status)
{
	StatusResponse response;
	if (request.m_username.empty() || request.m_passwd.empty())
	{
		status = BAD_REQUEST;
		response.m_error_msg = "Username or passwd is empty!";
		return response;
	}
	auto results = p_sqlExecuser->ExecuteQuery<User>(QueryUserByUsername, request.m_username);
	if (results.size() >= 1)
	{
		status = BAD_REQUEST;
		response.m_error_msg = "Username has been registered!";
		return response;
	}
	p_sqlExecuser->ExecuteUpdate(InsertUser, request.m_username, Sha256(request.m_passwd));
	status = SUCCESS;
	return response;
}

AuthResponse ChatService::LoginHandler(const Connection::Ptr& conn, const AuthRequest& request, Status& status)
{
	AuthResponse response;
	if (request.m_username.empty() || request.m_passwd.empty())
	{
		status = BAD_REQUEST;
		response.m_error_msg = "Username or passwd is empty!";
		return response;
	}
	auto results = p_sqlExecuser->ExecuteQuery<User>(QueryUserByUsername, request.m_username);

	if (results.size() < 1)
	{
		status = BAD_REQUEST;
		response.m_error_msg = "Username has not registered!";
		return response;
	}
	User user = results[0];
	{
		if (IsUserOnline(user.m_id))
		{
			status = BAD_REQUEST;
			response.m_error_msg = "User has online!";
			return response;
		}
	}
	if (user.m_passwd != Sha256(request.m_passwd))
	{
		status = BAD_REQUEST;
		response.m_error_msg = "Passwd error!";
		return response;
	}
	{
		std::lock_guard<std::mutex> lock(m_userConnMtx);
		m_userConnMap[user.m_id] = conn;
	}
	{
		std::lock_guard<std::mutex> lock(m_userMtx);
		user.m_state = UserState::ONLINE;
		m_userMap[user.m_id] = user;
	}
	response.m_user = user;
	response.m_chatRooms = p_sqlExecuser->ExecuteQuery<ChatRoom>(QueryChatRoomByUserId, user.m_id);
	response.m_friends = p_sqlExecuser->ExecuteQuery<User>(QueryFriendsByUserId, user.m_id);
	for (auto& frd : response.m_friends) {
		{
			frd.m_state = IsUserOnline(frd.m_id) ? m_userMap[frd.m_id].m_state : UserState::OFFLINE;
		}
	}
	try
	{
		p_sqlExecuser->BeginTransaction();
		response.m_offlineMessages = p_sqlExecuser->ExecuteQuery<Message>(QueryOfflineMsgsByUserId, user.m_id);
		p_sqlExecuser->ExecuteUpdate(DeleteOfflineMsgsByUserId, user.m_id); // todo client confirmation
		p_sqlExecuser->CommitTransaction();
	}
	catch (const std::exception& e)
	{
		p_sqlExecuser->RollbackTransaction();
		status = INTERNAL_ERROR;
		response.m_error_msg = "Server internal error!";
		LOG_ERROR("LoginHandler exception: {}", e.what());
		return response;
	}
	response.m_friendInvitations = p_sqlExecuser->ExecuteQuery<FriendInvitation>(QueryFriendsInvitationByFrdId, user.m_id);
	response.m_chatRoomInvitations = p_sqlExecuser->ExecuteQuery<ChatRoomInvitation>(QueryChatRoomInvitationByUId, user.m_id);
	status = SUCCESS;
	return response;
}

StatusResponse ChatService::LogoutHandler(const Connection::Ptr& conn, const UserIdRequest& request, Status& status) {
	StatusResponse response;
	if (!CheckAuth(request.m_userId, conn)) {
		status = UNAUTHORIZED;
		response.m_error_msg = "Unauthorized access!";
		return response;
	}
	{
		std::lock_guard<std::mutex> lock(m_userMtx);
		m_userMap.erase(request.m_userId);
	}
	{
		std::lock_guard<std::mutex> lock(m_userConnMtx);
		m_userConnMap.erase(request.m_userId);
	}
	status = SUCCESS;
	return response;
}

ChatRoomResponse ChatService::CreateChatRoomHandler(const Connection::Ptr& conn, const CreateChatRoomRequest& request, Status& status) {
	ChatRoomResponse response;
	if (!CheckAuth(request.m_ownerId, conn)) {
		status = UNAUTHORIZED;
		response.m_error_msg = "Unauthorized access!";
		return response;
	}
	if (request.m_name.empty()) {
		status = BAD_REQUEST;
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
	catch (const std::exception& e)
	{
		p_sqlExecuser->RollbackTransaction();
		status = INTERNAL_ERROR;
		response.m_error_msg = "Server internal error!";
		LOG_ERROR("CreateChatRoomHandler exception: {}", e.what());
	}
	status = SUCCESS;
	return response;
}

ChatRoomInvitationResponse ChatService::InviteToChatRoomHandler(const Connection::Ptr& conn, const InviteToChatRoomRequest& request, Status& status) {
	ChatRoomInvitationResponse response;
	if (!CheckAuth(request.m_inviterId, conn)) {
		status = UNAUTHORIZED;
		response.m_error_msg = "Unauthorized access!";
		return response;
	}
	auto results = p_sqlExecuser->ExecuteQuery<ChatRoomMember>(QueryChatRoomMemberByCrIdAndUId, request.m_roomId, request.m_inviteeId);
	if (results.size() > 0) {
		status = BAD_REQUEST;
		response.m_error_msg = "User has already in chat room!";
		return response;
	}
	p_sqlExecuser->BeginTransaction();
	try
	{
		auto member = p_sqlExecuser->ExecuteUpdateAndReturn<ChatRoomMember>
			(InsertChatRoomMember, request.m_roomId, request.m_inviteeId, ChatRoomRole::MEMBER, AddRelationStatus::PENDING, request.m_inviterId);
		response.m_invitation.m_id = member.m_id;
		{
			std::lock_guard<std::mutex> lock(m_userMtx);
			response.m_invitation.m_inviter = m_userMap[request.m_inviterId];
		}
		response.m_invitation.m_chatRoom = p_sqlExecuser->ExecuteQuery<ChatRoom>(QueryChatRoomByCrId, member.m_roomId)[0];
		if (auto inviteeConn = GetUserConnection(request.m_inviteeId))
		{
			PushOnlineData<ChatRoomInvitation>(inviteeConn, response.m_invitation, PushType::ChatRoomInvitation);
		}
		p_sqlExecuser->CommitTransaction();
	}
	catch (const std::exception& e)
	{
		p_sqlExecuser->RollbackTransaction();
		status = INTERNAL_ERROR;
		response.m_error_msg = "Server internal error!";
		LOG_ERROR("InvateToChatRoomHandler exception: {}", e.what());
		return response;
	}
	status = SUCCESS;
	return response;
}

ChatRoomResponse ChatService::AcceptChatRoomtInvitationHandler(const Connection::Ptr& conn, const HandleInvitationRequest& request, Status& status) {
	ChatRoomResponse response;
	if (!CheckAuth(request.m_userId, conn)) {
		status = UNAUTHORIZED;
		response.m_error_msg = "Unauthorized access!";
		return response;
	}
	auto results = p_sqlExecuser->ExecuteQuery<ChatRoomInvitation>(QueryChatRoomInvitationByIdAndUId, request.m_invitation_id, request.m_userId);
	if (results.size() < 1) {
		status = BAD_REQUEST;
		response.m_error_msg = "Invitation not found!";
		return response;
	}
	auto invitation = results[0];
	p_sqlExecuser->ExecuteUpdate(UpdateChatRoomMemberStatus, AddRelationStatus::ACCEPTED, request.m_invitation_id);
	response.m_chatRoom = p_sqlExecuser->ExecuteQuery<ChatRoom>(QueryChatRoomByCrId, invitation.m_chatRoom.m_id)[0];
	if (auto userConn = GetUserConnection(invitation.m_inviter.m_id))
	{
		ChatRoomInvitationDecision decision;
		decision.m_invitationId = invitation.m_id;
		decision.m_chatRoom = invitation.m_chatRoom;
		decision.m_status = AddRelationStatus::ACCEPTED;
		PushOnlineData<ChatRoomInvitationDecision>(userConn, decision, PushType::ChatRoomInvitationDecision);
	}
	status = SUCCESS;
	return response;
}

StatusResponse ChatService::RejectChatRoomInvitationHandler(const Connection::Ptr& conn, const HandleInvitationRequest& request, Status& status) {
	StatusResponse response;
	if (!CheckAuth(request.m_userId, conn)) {
		status = UNAUTHORIZED;
		response.m_error_msg = "Unauthorized access!";
		return response;
	}
	auto results = p_sqlExecuser->ExecuteQuery<ChatRoomInvitation>(QueryChatRoomInvitationByIdAndUId, request.m_invitation_id, request.m_userId);
	if (results.size() < 1) {
		status = BAD_REQUEST;
		response.m_error_msg = "Invitation not found!";
		return response;
	}
	auto invitation = results[0];
	p_sqlExecuser->ExecuteUpdate(UpdateChatRoomMemberStatus, AddRelationStatus::REJECTED, request.m_invitation_id);
	if (auto userConn = GetUserConnection(invitation.m_inviter.m_id))
	{
		ChatRoomInvitationDecision decision;
		decision.m_invitationId = invitation.m_id;
		decision.m_chatRoom = invitation.m_chatRoom;
		decision.m_status = AddRelationStatus::REJECTED;
		PushOnlineData<ChatRoomInvitationDecision>(userConn, decision, PushType::ChatRoomInvitationDecision);
	}
	status = SUCCESS;
	return response;
}

MessageResponse ChatService::OneChatHandler(const Connection::Ptr& conn, const OneChatRequest& request, Status& status) {
	MessageResponse response;
	if (!CheckAuth(request.m_senderId, conn)) {
		status = UNAUTHORIZED;
		response.m_error_msg = "Unauthorized access!";
		return response;
	}
	if (request.m_receiverId == request.m_senderId) {
		status = BAD_REQUEST;
		response.m_error_msg = "Cannot send message to self!";
		return response;
	}
	if (request.m_message.empty()) {
		status = BAD_REQUEST;
		response.m_error_msg = "Message is empty!";
		return response;
	}
	p_sqlExecuser->BeginTransaction();
	try
	{
		response.m_message = p_sqlExecuser->ExecuteUpdateAndReturn<Message>(InsertMessage, nullptr, request.m_senderId, request.m_message);
		if (auto receiverConn = GetUserConnection(request.m_receiverId))
		{
			PushOnlineData<Message>(receiverConn, response.m_message, PushType::Message);
		}
		else {
			p_sqlExecuser->ExecuteUpdate(InsertOfflineRecipient, response.m_message.m_id, request.m_receiverId);
		}
		p_sqlExecuser->CommitTransaction();
	}
	catch (const std::exception& e)
	{
		p_sqlExecuser->RollbackTransaction();
		status = INTERNAL_ERROR;
		response.m_error_msg = "Server internal error!";
		LOG_ERROR("OneChatHandler exception: {}", e.what());
		return response;
	}
	status = SUCCESS;
	return response;
}

MessageResponse ChatService::GroupChatHandler(const Connection::Ptr& conn, const GroupChatRequest& request, Status& status) {
	MessageResponse response;
	if (!CheckAuth(request.m_senderId, conn)) {
		status = UNAUTHORIZED;
		response.m_error_msg = "Unauthorized access!";
		return response;
	}
	if (request.m_message.empty()) {
		status = BAD_REQUEST;
		response.m_error_msg = "Message is empty!";
		return response;
	}
	p_sqlExecuser->BeginTransaction();
	try
	{
		response.m_message = p_sqlExecuser->ExecuteUpdateAndReturn<Message>(InsertMessage, request.m_roomId, request.m_senderId, request.m_message);
		bool isUserInChatRoom = false;
		auto members = p_sqlExecuser->ExecuteQuery<ChatRoomMember>(QueryChatRoomMemberByCrId, request.m_roomId);
		for (const auto& member : members) {
			if (member.m_userId == request.m_senderId) {
				isUserInChatRoom = true;
				continue;
			}
			if (auto receiverConn = GetUserConnection(member.m_userId))
			{
				PushOnlineData<Message>(receiverConn, response.m_message, PushType::Message);
			}
			else {
				p_sqlExecuser->ExecuteUpdate(InsertOfflineRecipient, response.m_message.m_id, member.m_userId);
			}
		}
		if (isUserInChatRoom)
		{
			p_sqlExecuser->CommitTransaction();
		}
		else {
			p_sqlExecuser->RollbackTransaction();
			status = FORBIDDEN;
			response.m_error_msg = "User not in group!";
			return response;
		}
	}
	catch (const std::exception& e)
	{
		p_sqlExecuser->RollbackTransaction();
		status = INTERNAL_ERROR;
		response.m_error_msg = "Server internal error!";
		LOG_ERROR("GroupChatHandler exception: {}", e.what());
		return response;
	}
	status = SUCCESS;
	return response;
}

FriendInvitationResponse ChatService::AddFriendHandler(const Connection::Ptr& conn, const AddFriendRequest& request, Status& status) {
	FriendInvitationResponse response;
	if (!CheckAuth(request.m_userId, conn)) {
		status = UNAUTHORIZED;
		response.m_error_msg = "Unauthorized access!";
		return response;
	}
	if (request.m_friendId == request.m_userId) {
		status = BAD_REQUEST;
		response.m_error_msg = "Cannot add self as friend!";
		return response;
	}
	auto results = p_sqlExecuser->ExecuteQuery<FriendRelation>(
		QueryFriendRelationsByUserIdAndFrdId, request.m_userId, request.m_friendId, request.m_friendId, request.m_userId);
	if (results.size() > 0 && results[0].m_status == AddRelationStatus::ACCEPTED) {
		status = BAD_REQUEST;
		response.m_error_msg = "Friend already exists!";
		return response;
	}
	if (results.size() > 0 && results[0].m_status == AddRelationStatus::PENDING) {
		status = BAD_REQUEST;
		response.m_error_msg = "Friend request is pending!";
		return response;
	}
	auto friendRlt = p_sqlExecuser->ExecuteUpdateAndReturn<FriendRelation>(InsertFriend, request.m_userId, request.m_friendId, AddRelationStatus::PENDING);
	response.m_invitation.m_id = friendRlt.m_id;
	{
		std::lock_guard<std::mutex> lock(m_userMtx);
		response.m_invitation.m_inviter = m_userMap[request.m_userId];
	}
	if (auto friendConn = GetUserConnection(request.m_friendId)) {
		PushOnlineData<FriendInvitation>(friendConn, response.m_invitation, PushType::FriendInvitation);
	}
	status = SUCCESS;
	return response;
}

UserResponse ChatService::AcceptFriendHandler(const Connection::Ptr& conn, const HandleInvitationRequest& request, Status& status) {
	UserResponse response;
	if (!CheckAuth(request.m_userId, conn)) {
		status = UNAUTHORIZED;
		response.m_error_msg = "Unauthorized access!";
		return response;
	}
	auto results = p_sqlExecuser->ExecuteQuery<FriendRelation>(QueryFriendRelationByIdAndUId, request.m_invitation_id, request.m_userId);
	if (results.size() < 1) {
		status = BAD_REQUEST;
		response.m_error_msg = "Invitation not found!";
		return response;
	}
	auto invitation = results[0];
	p_sqlExecuser->ExecuteUpdate(UpdateFriendStatus, AddRelationStatus::ACCEPTED, request.m_invitation_id);
	p_sqlExecuser->ExecuteUpdate(InsertFriend, invitation.m_friendId, invitation.m_userId, AddRelationStatus::ACCEPTED); //双向关系
	if (auto userConn = GetUserConnection(invitation.m_userId))
	{
		{
			std::lock_guard<std::mutex> lock(m_userMtx);
			response.m_user = m_userMap[invitation.m_userId];
		}
		FriendInvitationDecision decision;
		decision.m_invitationId = request.m_invitation_id;
		{
			std::lock_guard<std::mutex> lock(m_userMtx);
			decision.m_friend = m_userMap[request.m_userId];
		}
		decision.m_status = AddRelationStatus::ACCEPTED;
		PushOnlineData<FriendInvitationDecision>(userConn, decision, PushType::FriendInvitationDesicion);
	}
	else {
		response.m_user = p_sqlExecuser->ExecuteQuery<User>(QueryUserById, invitation.m_userId)[0];
	}
	status = SUCCESS;
	return response;
}

StatusResponse ChatService::RejectFriendHandler(const Connection::Ptr& conn, const HandleInvitationRequest& request, Status& status) {
	StatusResponse response;
	if (!CheckAuth(request.m_userId, conn)) {
		status = UNAUTHORIZED;
		response.m_error_msg = "Unauthorized access!";
		return response;
	}
	auto results = p_sqlExecuser->ExecuteQuery<FriendRelation>(QueryFriendRelationByIdAndUId, request.m_invitation_id, request.m_userId);
	if (results.size() < 1) {
		status = BAD_REQUEST;
		response.m_error_msg = "Invitation not found!";
		return response;
	}
	auto invitation = results[0];
	p_sqlExecuser->ExecuteUpdate(UpdateFriendStatus, AddRelationStatus::REJECTED, request.m_invitation_id);
	if (auto userConn = GetUserConnection(invitation.m_userId))
	{
		FriendInvitationDecision decision;
		decision.m_invitationId = request.m_invitation_id;
		{
			std::lock_guard<std::mutex> lock(m_userMtx);
			decision.m_friend = m_userMap[request.m_userId];
		}
		decision.m_status = AddRelationStatus::REJECTED;
		PushOnlineData<FriendInvitationDecision>(userConn, decision, PushType::FriendInvitationDesicion);
	}
	status = SUCCESS;
	return response;
}