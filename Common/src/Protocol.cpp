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

std::string ProtocolCodec::PackProtocolRequest(const ProtocolRequest::Ptr &msg)
{
    msg->m_magic = MAGIC;
    uint32_t checkSum = 0;
    uint32_t methodLength = static_cast<uint32_t>(msg->m_method.size());
    uint32_t tokenLength = static_cast<uint32_t>(msg->m_token.size());
    uint32_t bodyLength = static_cast<uint32_t>(msg->m_payload.size());

    std::string result;
    result.append(reinterpret_cast<const char *>(&msg->m_magic), sizeof(msg->m_magic));
    checkSum += CRC32::Instance().CalCheckSum(reinterpret_cast<const char *>(&msg->m_magic), sizeof(msg->m_magic));

    result.append(reinterpret_cast<const char *>(&methodLength), sizeof(methodLength));
    result.append(msg->m_method);
    checkSum += CRC32::Instance().CalCheckSum(msg->m_method.c_str(), methodLength);

    result.append(reinterpret_cast<const char *>(&tokenLength), sizeof(tokenLength));
    result.append(msg->m_token);
    checkSum += CRC32::Instance().CalCheckSum(msg->m_token.c_str(), tokenLength);

    result.append(reinterpret_cast<const char *>(&bodyLength), sizeof(bodyLength));
    result.append(msg->m_payload);
    checkSum += CRC32::Instance().CalCheckSum(msg->m_payload.c_str(), bodyLength);

    msg->m_checkSum = checkSum;
    result.append(reinterpret_cast<const char *>(&msg->m_checkSum), sizeof(msg->m_checkSum));
    return result;
}

std::string ProtocolCodec::PackProtocolResponse(const ProtocolResponse::Ptr &msg)
{
    msg->m_magic = MAGIC;
    uint32_t methodLength = static_cast<uint32_t>(msg->m_method.size());
    uint32_t bodyLength = static_cast<uint32_t>(msg->m_payload.size());
    uint32_t checkSum = 0;

    std::string result;
    result.append(reinterpret_cast<const char *>(&msg->m_magic), sizeof(msg->m_magic));
    checkSum += CRC32::Instance().CalCheckSum(reinterpret_cast<const char *>(&msg->m_magic), sizeof(msg->m_magic));

    result.append(reinterpret_cast<const char *>(&methodLength), sizeof(methodLength));
    result.append(msg->m_method);
    checkSum += CRC32::Instance().CalCheckSum(msg->m_method.c_str(), methodLength);

    result.append(reinterpret_cast<const char *>(&msg->m_status), sizeof(msg->m_status));

    result.append(reinterpret_cast<const char *>(&bodyLength), sizeof(bodyLength));
    result.append(msg->m_payload);
    checkSum += CRC32::Instance().CalCheckSum(msg->m_method.c_str(), methodLength);

    msg->m_checkSum = checkSum;
    result.append(reinterpret_cast<const char *>(&msg->m_checkSum), sizeof(msg->m_checkSum));
    return result;
}

std::vector<ProtocolRequest::Ptr> ProtocolCodec::UnPackProtocolRequest(std::string &buffer)
{
    std::vector<ProtocolRequest::Ptr> result;
    while (buffer.size() >= sizeof(uint32_t))
    {
        uint32_t magic;
        std::memcpy(&magic, buffer.data(), sizeof(magic));
        if (magic != MAGIC)
        {
            LOG_WARN("Invalid magic! expected = {}, actual = {}", MAGIC, magic);
            SyncToMagic(buffer);
            continue;
        }

        ProtocolRequest::Ptr msg = std::make_shared<ProtocolRequest>();
        size_t offset = sizeof(magic);
        uint32_t checkSum = CRC32::Instance().CalCheckSum(reinterpret_cast<const char *>(&magic), sizeof(magic));

        uint32_t methodLength;
        if (buffer.size() < offset + sizeof(methodLength))
            break;
        std::memcpy(&methodLength, buffer.data() + offset, sizeof(methodLength));
        offset += sizeof(methodLength);
        if (buffer.size() < offset + methodLength)
            break;
        msg->m_method.assign(buffer.data() + offset, methodLength);
        offset += methodLength;
        checkSum += CRC32::Instance().CalCheckSum(msg->m_method.c_str(), msg->m_method.size());

        uint32_t tokenLength;
        if (buffer.size() < offset + sizeof(tokenLength))
            break;
        std::memcpy(&tokenLength, buffer.data() + offset, sizeof(tokenLength));
        offset += sizeof(tokenLength);
        if (buffer.size() < offset + tokenLength)
            break;
        msg->m_token.assign(buffer.data() + offset, tokenLength);
        offset += tokenLength;
        checkSum += CRC32::Instance().CalCheckSum(msg->m_token.c_str(), msg->m_token.size());

        uint32_t bodyLength;
        if (buffer.size() < offset + sizeof(bodyLength))
            break;
        std::memcpy(&bodyLength, buffer.data() + offset, sizeof(bodyLength));
        offset += sizeof(bodyLength);
        if (buffer.size() < offset + bodyLength)
            break;
        msg->m_payload.assign(buffer.data() + offset, bodyLength);
        offset += bodyLength;
        checkSum += CRC32::Instance().CalCheckSum(msg->m_payload.c_str(), msg->m_payload.size());

        if (buffer.size() < offset + sizeof(msg->m_checkSum))
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
        buffer.erase(0, offset);
    }
    return result;
}

std::vector<ProtocolResponse::Ptr> ProtocolCodec::UnPackProtocolResponse(std::string &buffer)
{
    std::vector<ProtocolResponse::Ptr> result;
    while (buffer.size() >= sizeof(uint32_t))
    {
        uint32_t magic;
        std::memcpy(&magic, buffer.data(), sizeof(magic));
        if (magic != MAGIC)
        {
            LOG_WARN("Invalid magic! expected = {}, actual = {}", MAGIC, magic);
            SyncToMagic(buffer);
            continue;
        }

        ProtocolResponse::Ptr msg = std::make_shared<ProtocolResponse>();
        size_t offset = sizeof(magic);
        uint32_t checkSum = CRC32::Instance().CalCheckSum(reinterpret_cast<const char *>(&magic), sizeof(magic));

        uint32_t methodLength;
        if (buffer.size() < offset + sizeof(methodLength))
            break;
        std::memcpy(&methodLength, buffer.data() + offset, sizeof(methodLength));
        offset += sizeof(methodLength);
        if (buffer.size() < offset + methodLength)
            break;
        msg->m_method.assign(buffer.data() + offset, methodLength);
        offset += methodLength;
        checkSum += CRC32::Instance().CalCheckSum(msg->m_method.c_str(), msg->m_method.size());

        if (buffer.size() < offset + sizeof(msg->m_status))
            break;
        std::memcpy(&msg->m_status, buffer.data() + offset, sizeof(msg->m_status));
        offset += sizeof(msg->m_status);

        uint32_t bodyLength;
        if (buffer.size() < offset + sizeof(bodyLength))
            break;
        std::memcpy(&bodyLength, buffer.data() + offset, sizeof(bodyLength));
        offset += sizeof(bodyLength);
        if (buffer.size() < offset + bodyLength)
            break;
        msg->m_payload.assign(buffer.data() + offset, bodyLength);
        offset += bodyLength;
        checkSum += CRC32::Instance().CalCheckSum(msg->m_payload.c_str(), msg->m_payload.size());

        if (buffer.size() < offset + sizeof(msg->m_checkSum))
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
        buffer.erase(0, offset);
    }
    return result;
}