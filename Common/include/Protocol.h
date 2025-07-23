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

struct alignas(8) ProtocolHeader
{
    using Ptr = std::shared_ptr<ProtocolHeader>;
    uint32_t m_magic = MAGIC;
    MethodType m_methodType;
    uint32_t m_bodyLength = 0;
    uint32_t m_sessionId;
    uint32_t m_checkSum;
};

struct ProtocolMessage
{
    using Ptr = std::shared_ptr<ProtocolMessage>;
    ProtocolHeader m_header;
    std::string m_payload;
};

class ProtocolCodec : public UncopybleAndUnmovable
{
public:
    explicit ProtocolCodec() = default;

    std::string PackProtocolMessage(const ProtocolMessage::Ptr &msg);

    std::vector<ProtocolMessage::Ptr> UnpackProtocolMessage(std::string &buffer);

private:
    void SyncToMagic(std::string &buffer);
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