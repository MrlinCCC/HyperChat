#include "Protocol.h"

std::string ProtocolCodec::PackProtocolMessage(const ProtocolMessage::Ptr &msg)
{
    ProtocolHeader &header = msg->m_header;
    header.m_magic = MAGIC;
    header.m_bodyLength = static_cast<uint32_t>(msg->m_payload.size());

    header.m_checkSum = 0;
    size_t headerWithoutChecksumSize = offsetof(ProtocolHeader, m_checkSum);
    header.m_checkSum = CRC32::Instance().CalCheckSum(reinterpret_cast<const char *>(&header), headerWithoutChecksumSize);
    header.m_checkSum += CRC32::Instance().CalCheckSum(msg->m_payload.data(), msg->m_payload.size());

    std::string result;
    result.append(reinterpret_cast<const char *>(&header), sizeof(header));
    result.append(msg->m_payload);
    return result;
}

std::vector<ProtocolMessage::Ptr> ProtocolCodec::UnpackProtocolMessage(std::string &buffer)
{
    std::vector<ProtocolMessage::Ptr> result;
    while (buffer.size() >= sizeof(ProtocolHeader))
    {
        ProtocolHeader header;
        std::memcpy(&header, buffer.data(), sizeof(header));

        if (header.m_magic != MAGIC)
        {
            LOG_WARN("Invalid magic! expected = {}, actual = {}", MAGIC, header.m_magic);
            SyncToMagic(buffer);
            continue;
        }

        if (sizeof(ProtocolHeader) + header.m_bodyLength > buffer.size())
        {
            LOG_WARN("Unsafe bodyLength! max = {}, actual = {}", buffer.size(), header.m_bodyLength);
            SyncToMagic(buffer);
            continue;
        }

        if (buffer.size() < sizeof(ProtocolHeader) + header.m_bodyLength)
            break;

        auto msg = std::make_shared<ProtocolMessage>();
        msg->m_header = header;

        size_t headerWithoutChecksumSize = offsetof(ProtocolHeader, m_checkSum);
        uint32_t actual = CRC32::Instance().CalCheckSum(reinterpret_cast<const char *>(&header), headerWithoutChecksumSize);

        if (header.m_bodyLength > 0)
        {
            msg->m_payload.assign(buffer.data() + sizeof(header), header.m_bodyLength);
            actual += CRC32::Instance().CalCheckSum(msg->m_payload.data(), msg->m_payload.size());
        }

        if (actual == header.m_checkSum)
        {
            result.push_back(msg);
        }
        else
        {
            LOG_WARN("Checksum failed! expected = {}, actual = {}", header.m_checkSum, actual);
        }

        buffer.erase(0, sizeof(header) + header.m_bodyLength);
    }

    return result;
}

void ProtocolCodec::SyncToMagic(std::string &buffer)
{
    for (size_t i = 0; i + sizeof(uint32_t) <= buffer.size(); ++i)
    {
        uint32_t magic;
        std::memcpy(&magic, buffer.data() + i, sizeof(magic));
        if (magic == MAGIC)
        {
            if (i > 0)
                buffer.erase(0, i);
            return;
        }
    }
    buffer.clear();
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