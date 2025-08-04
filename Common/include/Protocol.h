#pragma once
#include <vector>
#include <memory>
#include <cstring>
#include <cstdint>
#include "Logger.h"

#define MAGIC 0xA1B2C3D4

// protocol
// enum MethodType : uint32_t
// {
//     REGISTER,
//     LOGIN,
//     LOGOUT,
//     ONE_CHAT,
//     GROUP_CHAT
// };

enum Status : uint32_t
{
    SUCCESS,
    INTERNAL_ERROR,
    NOT_FOUND,
    BAD_REQUEST,
    UNAUTHORIZED,
    PERMISSION_DENIED
};

struct ProtocolRequest
{
    using Ptr = std::shared_ptr<ProtocolRequest>;
    uint32_t m_magic = MAGIC;
    std::string m_method;
    std::string m_token;
    std::string m_payload;
    uint32_t m_checkSum;
};

struct ProtocolResponse
{
    using Ptr = std::shared_ptr<ProtocolResponse>;
    uint32_t m_magic = MAGIC;
    std::string m_method;
    Status m_status;
    std::string m_payload;
    uint32_t m_checkSum;
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
    ProtocolCodec() = default;

    std::string PackProtocolRequest(const ProtocolRequest::Ptr &msg);
    std::string PackProtocolResponse(const ProtocolResponse::Ptr &msg);

    std::vector<ProtocolRequest::Ptr> UnPackProtocolRequest(std::string &buffer);
    std::vector<ProtocolResponse::Ptr> UnPackProtocolResponse(std::string &buffer);

private:
    void SyncToMagic(std::string &buffer);
};
