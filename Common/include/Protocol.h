#pragma once
#include <vector>
#include <memory>
#include <cstring>
#include <cstdint>
#include "Logger.h"

#define MAGIC 0xA1B2C3D4

enum Status : uint32_t
{
	SUCCESS,
	INTERNAL_ERROR,
	NOT_FOUND,
	BAD_REQUEST,
	UNAUTHORIZED,
	FORBIDDEN
};

struct ProtocolRequest
{
	using Ptr = std::shared_ptr<ProtocolRequest>;
	uint32_t m_magic = MAGIC;
	uint32_t m_requestId;
	std::string m_method;
	std::string m_payload;
	uint32_t m_checkSum;
};

struct ProtocolResponse
{
	using Ptr = std::shared_ptr<ProtocolResponse>;
	uint32_t m_magic = MAGIC;
	uint32_t m_requestId;
	std::string m_pushType;
	Status m_status;
	std::string m_payload;
	uint32_t m_checkSum;
};

class CRC32 : public UncopybleAndUnmovable
{
public:
	static CRC32& Instance();

	uint32_t CalCheckSum(const char* data, size_t length) const;

private:
	CRC32();

	void Generate_table();

	uint32_t m_table[256];
	const uint32_t m_polynomial = 0x04C11DB7;
};

class ProtocolCodec : public UncopybleAndUnmovable
{
public:
	static ProtocolCodec& Instance();

	std::string PackProtocolRequest(const ProtocolRequest::Ptr& msg);
	std::string PackProtocolResponse(const ProtocolResponse::Ptr& msg);

	std::vector<ProtocolRequest::Ptr> UnPackProtocolRequest(std::vector<char>& buffer);
	std::vector<ProtocolResponse::Ptr> UnPackProtocolResponse(std::vector<char>& buffer);

private:
	ProtocolCodec() = default;
	void SyncToMagic(std::vector<char>& buffer);
};
