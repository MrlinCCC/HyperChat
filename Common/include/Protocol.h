#pragma once
#include <vector>
#include <memory>
#include "Logger.h"
#include <algorithm>
#include "Singleton.h"

constexpr const size_t ProtocolBufferSize = 1024;

class CRC32 : public Singleton<CRC32>
{
	friend class Singleton<CRC32>;

public:
	uint32_t CalCheckSum(const char *data, size_t length) const;

private:
	CRC32();

	void Generate_table();

	uint32_t m_table[256];
	const uint32_t m_polynomial = 0x04C11DB7;
};

class DynamicRingBuffer
{
public:
	explicit DynamicRingBuffer(size_t capacity);
	~DynamicRingBuffer();

	inline size_t Size() const { return m_size; }
	inline size_t Capacity() const { return m_capacity; }
	inline bool Empty() const { return m_size == 0; }

	void Write(const char *data, size_t len);
	size_t Read(char *dst, size_t len);
	size_t PeekAt(size_t pos, char *dst, size_t len) const;
	void Consume(size_t len);

private:
	void CheckCapacity(size_t needed);

	char *m_buf;
	size_t m_capacity;
	size_t m_head, m_tail, m_size;
};

constexpr uint32_t Magic = 0xA1B2C3D4;

enum class Status : uint8_t
{
	SUCCESS,
	INTERNAL_ERROR,
	NOT_FOUND,
	BAD_REQUEST,
	UNAUTHORIZED,
	FORBIDDEN,
	TIMEOUT
};

enum class FrameType : uint8_t
{
	REQUEST,
	RESPONSE,
	PUSH,
	// PUSH_ACK, 聊天室场景不保证消息的可靠性
	PING,
	PONG
};

enum class MethodType : uint16_t
{
	REGISTER = 0,
	LOGIN,
	LOGOUT,
	CREATE_CHAT_ROOM,
	INVITE_TO_CHAT_ROOM,
	ACCEPT_CHAT_ROOM_INVITATION,
	REJECT_CHAT_ROOM_INVITATION,
	ONE_CHAT,
	GROUP_CHAT,
	ADD_FRIEND,
	ACCEPT_FRIEND_INVITATION,
	REJECT_FRIEND_INVITATION,

	PUSH_MESSAGE = 0x100,
	PUSH_CHAT_ROOM_INVITATION,
	PUSH_CHAT_ROOM_INVITATION_DECISION,
	PUSH_FRIEND_INVITATION,
	PUSH_FRIEND_INVITATION_DECISION
};

struct ProtocolHeader
{
	using Ptr = std::shared_ptr<ProtocolHeader>;

	uint32_t m_magic;
	uint32_t m_requestId;
	MethodType m_method;
	FrameType m_type;
	Status m_status;
	uint32_t m_bodyLength;

	std::vector<char> Serialize() const
	{
		std::vector<char> buffer;
		buffer.reserve(sizeof(ProtocolHeader));

		auto append = [&](auto value)
		{
			char *p = reinterpret_cast<char *>(&value);
			buffer.insert(buffer.end(), p, p + sizeof(value));
		};

		append(m_magic);
		append(m_requestId);
		append(m_type);
		append(m_method);
		append(m_status);
		append(m_bodyLength);

		return buffer;
	}

	void DeSerialize(const std::vector<char> &buf, size_t offset)
	{
		auto read = [&](auto &field)
		{
			using T = std::decay_t<decltype(field)>;
			if (offset + sizeof(T) > buf.size())
				throw std::runtime_error("Buffer too small");
			std::memcpy(&field, buf.data() + offset, sizeof(T));
			offset += sizeof(T);
		};

		read(m_magic);
		read(m_requestId);
		read(m_type);
		read(m_method);
		read(m_status);
		read(m_bodyLength);
	}
};

struct ProtocolFrame
{
	using Ptr = std::shared_ptr<ProtocolFrame>;

	ProtocolHeader m_header;
	std::string m_payload;
	uint32_t m_checkSum;

	ProtocolFrame() = default;

	ProtocolFrame(FrameType type)
	{
		m_header.m_type = type;
	}

	ProtocolFrame(FrameType type, MethodType method, uint32_t requestId, Status status = Status::SUCCESS, std::string payload = "")
	{
		m_header.m_type = type;
		m_header.m_method = method;
		m_header.m_requestId = requestId;
		m_header.m_status = status;
		m_payload = payload;
	}
};

class ProtocolCodec
{
public:
	ProtocolCodec();

	std::string PackProtocolFrame(const ProtocolFrame::Ptr &frame);

	std::vector<ProtocolFrame::Ptr> UnPackProtocolFrame(const char *buffer, size_t length);

private:
	void SyncToMagic();
	DynamicRingBuffer m_buffer;
};
