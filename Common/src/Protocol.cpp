#include "Protocol.h"

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