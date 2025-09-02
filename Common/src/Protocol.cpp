#include "Protocol.h"
#include "Serializer.hpp"

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

DynamicRingBuffer::DynamicRingBuffer(size_t capacity)
	: m_capacity(capacity),
	  m_buf(new char[capacity]),
	  m_head(0), m_tail(0), m_size(0) {}

DynamicRingBuffer::~DynamicRingBuffer()
{
	if (m_buf)
	{
		delete[] m_buf;
	}
}

void DynamicRingBuffer::CheckCapacity(size_t needed)
{
	if (needed <= m_capacity)
		return;

	size_t newCap = (std::max)(needed, m_capacity * 2);
	char *newBuf = new char[newCap];

	if (m_size > 0)
	{
		if (m_head < m_tail)
		{
			std::memcpy(newBuf, m_buf + m_head, m_size);
		}
		else
		{
			size_t firstPart = m_capacity - m_head;
			std::memcpy(newBuf, m_buf + m_head, firstPart);
			std::memcpy(newBuf + firstPart, m_buf, m_tail);
		}
	}
	delete[] m_buf;
	m_buf = newBuf;
	m_capacity = newCap;
	m_head = 0;
	m_tail = m_size;
}

void DynamicRingBuffer::Write(const char *data, size_t len)
{
	CheckCapacity(m_size + len);

	size_t firstPart = (std::min)(len, m_capacity - m_tail);
	std::memcpy(m_buf + m_tail, data, firstPart);

	size_t secondPart = len - firstPart;
	if (secondPart > 0)
		std::memcpy(m_buf, data + firstPart, secondPart);

	m_tail = (m_tail + len) % m_capacity;
	m_size += len;
}

size_t DynamicRingBuffer::Read(char *dst, size_t len)
{
	size_t n = (std::min)(len, m_size);
	if (n == 0)
		return 0;

	size_t firstPart = (std::min)(n, m_capacity - m_head);
	std::memcpy(dst, m_buf + m_head, firstPart);

	size_t secondPart = n - firstPart;
	if (secondPart > 0)
		std::memcpy(dst + firstPart, m_buf, secondPart);

	m_head = (m_head + n) % m_capacity;
	m_size -= n;

	return n;
}

size_t DynamicRingBuffer::PeekAt(size_t pos, char *dst, size_t len) const
{
	if (pos >= m_size)
		return 0;
	size_t n = (std::min)(len, m_size - pos);

	size_t start = (m_head + pos) % m_capacity;

	size_t firstPart = (std::min)(n, m_capacity - start);
	std::memcpy(dst, m_buf + start, firstPart);

	size_t secondPart = n - firstPart;
	if (secondPart > 0)
		std::memcpy(dst + firstPart, m_buf, secondPart);

	return n;
}

void DynamicRingBuffer::Consume(size_t len)
{
	if (len >= m_size)
	{
		m_head = m_tail = m_size = 0;
		return;
	}

	m_head = (m_head + len) % m_capacity;
	m_size -= len;
}

ProtocolCodec::ProtocolCodec() : m_buffer(ProtocolBufferSize)
{
}

void ProtocolCodec::SyncToMagic()
{
	size_t size = m_buffer.Size();
	if (size < sizeof(uint32_t))
		return;

	std::vector<char> data(size);
	m_buffer.PeekAt(0, data.data(), size);

	const char *magicBytes = reinterpret_cast<const char *>(&Magic);
	auto it = std::search(
		data.begin() + 1, data.end(),
		magicBytes, magicBytes + sizeof(uint32_t));

	if (it != data.end())
	{
		size_t offset = std::distance(data.begin(), it);
		m_buffer.Consume(offset);
	}
	else
	{
		m_buffer.Consume(size);
	}
}

std::string ProtocolCodec::PackProtocolFrame(const ProtocolFrame::Ptr &frame)
{
	frame->m_header.m_magic = Magic;
	frame->m_header.m_bodyLength = static_cast<uint32_t>(frame->m_payload.size());
	std::string result = Serializer::Serialize(frame->m_header);
	result += frame->m_payload;
	uint32_t checkSum = CRC32::Instance().CalCheckSum(result.data(), result.size());
	result.append(reinterpret_cast<const char *>(&checkSum), sizeof(checkSum));
	return result;
}

std::vector<ProtocolFrame::Ptr> ProtocolCodec::UnPackProtocolFrame(const char *buffer, size_t length)
{
	std::vector<ProtocolFrame::Ptr> frames;
	m_buffer.Write(buffer, length);
	uint32_t headerSize = sizeof(ProtocolHeader);
	while (headerSize <= m_buffer.Size())
	{
		uint32_t magic = 0;
		m_buffer.PeekAt(0, reinterpret_cast<char *>(&magic), 4);
		if (magic != Magic)
		{
			LOG_WARN("Invalid magic! expected = {}, actual = {}", Magic, magic);
			SyncToMagic();
			continue;
		}
		uint32_t bodyLength = 0;
		m_buffer.PeekAt(headerSize - 4, reinterpret_cast<char *>(&bodyLength), 4);
		if (headerSize + bodyLength + 4 > m_buffer.Size())
			break;
		std::string frameStr(headerSize + bodyLength, '\0');
		m_buffer.Read(frameStr.data(), headerSize + bodyLength);
		uint32_t checkSum = 0;
		m_buffer.Read(reinterpret_cast<char *>(&checkSum), 4);
		uint32_t calCheckSum = CRC32::Instance().CalCheckSum(frameStr.data(), headerSize + bodyLength);
		if (checkSum == calCheckSum)
		{
			ProtocolFrame::Ptr frame = std::make_shared<ProtocolFrame>();
			frame->m_header = Serializer::DeSerialize<ProtocolHeader>(std::string(frameStr.begin(), frameStr.begin() + headerSize));
			frame->m_payload = std::string(frameStr.begin() + headerSize, frameStr.end());
			frame->m_checkSum = checkSum;
			frames.push_back(std::move(frame));
		}
		else
			LOG_WARN("Checksum failed! expected = {}, actual = {}", checkSum, calCheckSum);
	}
	return frames;
}