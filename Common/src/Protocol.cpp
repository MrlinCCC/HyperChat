#include "Protocol.h"

void ProtocolCodec::SyncToMagic(std::vector<char> &buffer, std::size_t &length)
{
	for (size_t i = 0; i + sizeof(uint32_t) <= length; ++i)
	{
		uint32_t magic;
		std::memcpy(&magic, buffer.data() + i, sizeof(magic));
		if (magic == MAGIC)
		{
			if (i > 0)
			{
				buffer.erase(buffer.begin(), buffer.begin() + i);
				length -= i;
			}
			return;
		}
	}
	buffer.clear();
	length = 0;
}

CRC32 &CRC32::Instance()
{
	static CRC32 instance;
	return instance;
}

CRC32::CRC32()
{
	Generate_table();
}

void CRC32::Generate_table()
{
	for (uint32_t i = 0; i < 256; ++i)
	{
		uint32_t crc = i << 24;
		for (int j = 0; j < 8; ++j)
		{
			if (crc & 0x80000000)
				crc = (crc << 1) ^ m_polynomial;
			else
				crc <<= 1;
		}
		m_table[i] = crc;
	}
}

uint32_t CRC32::CalCheckSum(const char *data, size_t length) const
{
	uint32_t crc = 0xFFFFFFFF;
	for (size_t i = 0; i < length; ++i)
	{
		uint8_t index = (crc >> 24) ^ static_cast<uint8_t>(data[i]);
		crc = (crc << 8) ^ m_table[index];
	}
	return crc ^ 0xFFFFFFFF;
}

ProtocolCodec &ProtocolCodec::Instance()
{
	static ProtocolCodec instance;
	return instance;
}

std::string ProtocolCodec::PackProtocolRequest(const ProtocolRequest::Ptr &msg)
{
	msg->m_magic = MAGIC;
	uint32_t methodLength = static_cast<uint32_t>(msg->m_method.size());
	uint32_t bodyLength = static_cast<uint32_t>(msg->m_payload.size());

	std::string result;
	result.append(reinterpret_cast<const char *>(&msg->m_magic), sizeof(msg->m_magic));
	result.append(reinterpret_cast<const char *>(&msg->m_requestId), sizeof(msg->m_requestId));
	result.append(reinterpret_cast<const char *>(&methodLength), sizeof(methodLength));
	result.append(msg->m_method);
	result.append(reinterpret_cast<const char *>(&bodyLength), sizeof(bodyLength));
	result.append(msg->m_payload);
	msg->m_checkSum = CRC32::Instance().CalCheckSum(result.data(), result.size());
	result.append(reinterpret_cast<const char *>(&msg->m_checkSum), sizeof(msg->m_checkSum));
	return result;
}

std::string ProtocolCodec::PackProtocolResponse(const ProtocolResponse::Ptr &msg)
{
	msg->m_magic = MAGIC;
	uint32_t bodyLength = static_cast<uint32_t>(msg->m_payload.size());
	uint32_t pushTypeLength = static_cast<uint32_t>(msg->m_pushType.size());

	std::string result;
	result.append(reinterpret_cast<const char *>(&msg->m_magic), sizeof(msg->m_magic));
	result.append(reinterpret_cast<const char *>(&msg->m_requestId), sizeof(msg->m_requestId));
	result.append(reinterpret_cast<const char *>(&pushTypeLength), sizeof(pushTypeLength));
	result.append(msg->m_pushType);
	result.append(reinterpret_cast<const char *>(&msg->m_status), sizeof(msg->m_status));
	result.append(reinterpret_cast<const char *>(&bodyLength), sizeof(bodyLength));
	result.append(msg->m_payload);

	msg->m_checkSum = CRC32::Instance().CalCheckSum(result.data(), result.size());
	result.append(reinterpret_cast<const char *>(&msg->m_checkSum), sizeof(msg->m_checkSum));
	return result;
}

std::vector<ProtocolRequest::Ptr> ProtocolCodec::UnPackProtocolRequest(std::vector<char> &buffer, std::size_t length)
{
	std::vector<ProtocolRequest::Ptr> result;
	while (length >= sizeof(uint32_t))
	{
		uint32_t magic;
		std::memcpy(&magic, buffer.data(), sizeof(magic));
		if (magic != MAGIC)
		{
			LOG_WARN("Invalid magic! expected = {}, actual = {}", MAGIC, magic);
			SyncToMagic(buffer, length);
			continue;
		}

		ProtocolRequest::Ptr msg = std::make_shared<ProtocolRequest>();
		size_t offset = sizeof(magic);

		if (length < offset + sizeof(msg->m_requestId))
			break;
		std::memcpy(&msg->m_requestId, buffer.data() + offset, sizeof(msg->m_requestId));
		offset += sizeof(msg->m_requestId);

		uint32_t methodLength;
		if (length < offset + sizeof(methodLength))
			break;
		std::memcpy(&methodLength, buffer.data() + offset, sizeof(methodLength));
		offset += sizeof(methodLength);
		if (length < offset + methodLength)
			break;
		msg->m_method.assign(buffer.data() + offset, methodLength);
		offset += methodLength;

		uint32_t bodyLength;
		if (length < offset + sizeof(bodyLength))
			break;
		std::memcpy(&bodyLength, buffer.data() + offset, sizeof(bodyLength));
		offset += sizeof(bodyLength);
		if (length < offset + bodyLength)
			break;
		msg->m_payload.assign(buffer.data() + offset, bodyLength);
		offset += bodyLength;

		uint32_t checkSum = CRC32::Instance().CalCheckSum(buffer.data(), offset);
		if (length < offset + sizeof(msg->m_checkSum))
			break;
		std::memcpy(&msg->m_checkSum, buffer.data() + offset, sizeof(msg->m_checkSum));
		offset += sizeof(msg->m_checkSum);

		if (checkSum == msg->m_checkSum)
		{
			result.push_back(msg);
		}
		else
		{
			LOG_WARN("Checksum failed! expected = {}, actual = {}", msg->m_checkSum, checkSum);
		}
		buffer.erase(buffer.begin(), buffer.begin() + offset);
		length -= offset;
	}
	return result;
}

std::vector<ProtocolResponse::Ptr> ProtocolCodec::UnPackProtocolResponse(std::vector<char> &buffer, std::size_t length)
{
	std::vector<ProtocolResponse::Ptr> result;
	while (length >= sizeof(uint32_t))
	{
		uint32_t magic;
		std::memcpy(&magic, buffer.data(), sizeof(magic));
		if (magic != MAGIC)
		{
			LOG_WARN("Invalid magic! expected = {}, actual = {}", MAGIC, magic);
			SyncToMagic(buffer, length);
			continue;
		}

		ProtocolResponse::Ptr msg = std::make_shared<ProtocolResponse>();
		size_t offset = sizeof(magic);

		if (length < offset + sizeof(msg->m_requestId))
			break;
		std::memcpy(&msg->m_requestId, buffer.data() + offset, sizeof(msg->m_requestId));
		offset += sizeof(msg->m_requestId);

		uint32_t pushTypeLength;
		if (length < offset + sizeof(pushTypeLength))
			break;
		std::memcpy(&pushTypeLength, buffer.data() + offset, sizeof(pushTypeLength));
		offset += sizeof(pushTypeLength);
		if (length < offset + pushTypeLength)
			break;
		msg->m_pushType.assign(buffer.data() + offset, pushTypeLength);
		offset += pushTypeLength;

		if (length < offset + sizeof(msg->m_status))
			break;
		std::memcpy(&msg->m_status, buffer.data() + offset, sizeof(msg->m_status));
		offset += sizeof(msg->m_status);

		uint32_t bodyLength;
		if (length < offset + sizeof(bodyLength))
			break;
		std::memcpy(&bodyLength, buffer.data() + offset, sizeof(bodyLength));
		offset += sizeof(bodyLength);
		if (length < offset + bodyLength)
			break;
		msg->m_payload.assign(buffer.data() + offset, bodyLength);
		offset += bodyLength;

		uint32_t checkSum = CRC32::Instance().CalCheckSum(buffer.data(), offset);
		if (length < offset + sizeof(msg->m_checkSum))
			break;
		std::memcpy(&msg->m_checkSum, buffer.data() + offset, sizeof(msg->m_checkSum));
		offset += sizeof(msg->m_checkSum);

		if (checkSum == msg->m_checkSum)
		{
			result.push_back(msg);
		}
		else
		{
			LOG_WARN("Checksum failed! expected = {}, actual = {}", msg->m_checkSum, checkSum);
		}
		buffer.erase(buffer.begin(), buffer.begin() + offset);
		length -= offset;
	}
	return result;
}