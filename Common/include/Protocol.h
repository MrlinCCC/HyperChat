#pragma once
#include <vector>
#include <memory>
#include <cstring>
#include <cstdint>
#include "Logger.h"

#define MAGIC 0xA1B2C3D4

// protocol
enum MethodType : uint32_t
{
    REGISTER,
    LOGIN,
    LOGOUT,
    ONE_CHAT,
    GROUP_CHAT
};

enum Status : uint32_t
{
    SUCCESS,
    INTERNAL_ERROR,
    NOT_FOUND,
    BAD_REQUEST,
    UNAUTHORIZED,
    PERMISSION_DENIED
};

struct alignas(4) ProtocolRequestHeader
{
    using Ptr = std::shared_ptr<ProtocolRequestHeader>;
    uint32_t m_magic = MAGIC;
    MethodType m_methodType;
    uint32_t m_bodyLength = 0;
    uint32_t m_sessionId;
    uint32_t m_checkSum;
};

struct alignas(4) ProtocolResponseHeader
{
    using Ptr = std::shared_ptr<ProtocolResponseHeader>;
    uint32_t m_magic = MAGIC;
    MethodType m_methodType;
    Status m_status;
    uint32_t m_bodyLength = 0;
    uint32_t m_checkSum;
};

struct ProtocolRequestMessage
{
    using Ptr = std::shared_ptr<ProtocolRequestMessage>;
    using Header = ProtocolRequestHeader;
    ProtocolRequestHeader m_header;
    std::string m_payload;
};

struct ProtocolResponseMessage
{
    using Ptr = std::shared_ptr<ProtocolResponseMessage>;
    using Header = ProtocolResponseHeader;
    ProtocolResponseHeader m_header;
    std::string m_payload;
};

class CRC32 : public UncopybleAndUnmovable
{
public:
    static CRC32 &Instance();

    uint32_t CalCheckSum(const char *data, size_t length) const;

private:
    CRC32();
    ~CRC32() = default;

    void Generate_table();

    uint32_t m_table[256];
    const uint32_t m_polynomial = 0x04C11DB7;
};

class ProtocolCodec : public UncopybleAndUnmovable
{
public:
    explicit ProtocolCodec() = default;

    template <typename ProtocolMessageT>
    std::string PackProtocolMessage(const typename ProtocolMessageT::Ptr &msg)
    {
        using Header = typename ProtocolMessageT::Header;
        Header &header = msg->m_header;
        header.m_magic = MAGIC;
        header.m_bodyLength = static_cast<uint32_t>(msg->m_payload.size());

        header.m_checkSum = 0;
        size_t headerWithoutChecksumSize = offsetof(Header, m_checkSum);
        header.m_checkSum = CRC32::Instance().CalCheckSum(reinterpret_cast<const char *>(&header), headerWithoutChecksumSize);
        header.m_checkSum += CRC32::Instance().CalCheckSum(msg->m_payload.data(), msg->m_payload.size());

        std::string result;
        result.append(reinterpret_cast<const char *>(&header), sizeof(header));
        result.append(msg->m_payload);
        return result;
    }

    template <typename ProtocolMessageT>
    std::vector<typename ProtocolMessageT::Ptr> UnpackProtocolMessage(std::string &buffer)
    {
        using Header = typename ProtocolMessageT::Header;
        std::vector<typename ProtocolMessageT::Ptr> result;
        while (buffer.size() >= sizeof(Header))
        {
            Header header;
            std::memcpy(&header, buffer.data(), sizeof(header));

            if (header.m_magic != MAGIC)
            {
                LOG_WARN("Invalid magic! expected = {}, actual = {}", MAGIC, header.m_magic);
                SyncToMagic(buffer);
                continue;
            }

            if (sizeof(Header) + header.m_bodyLength > buffer.size())
            {
                LOG_WARN("Unsafe bodyLength! max = {}, actual = {}", buffer.size(), header.m_bodyLength);
                SyncToMagic(buffer);
                continue;
            }

            if (buffer.size() < sizeof(Header) + header.m_bodyLength)
                break;

            auto msg = std::make_shared<ProtocolMessageT>();
            msg->m_header = header;

            size_t headerWithoutChecksumSize = offsetof(Header, m_checkSum);
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

private:
    void SyncToMagic(std::string &buffer);
};
