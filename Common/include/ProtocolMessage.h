#pragma once
#include <iostream>
#include <json/json.h>

enum BodyType : uint16_t
{
    JSON,
    //...
};

enum MessageType : uint16_t
{
    LOGIN,
    CHAT,
    //...
};

struct MessageHeader
{
    uint32_t m_magic = 0xA1B2C3D4;
    uint32_t m_bodyLength;
    uint32_t m_sessionId;
    BodyType m_bodyType;
    MessageType m_msgType;
};

struct MessageBody
{
    using ptr = std::shared_ptr<MessageBody>;
    virtual std::string BodySerialize() const = 0;
    virtual MessageBody::ptr BodyDeSerialize(const std::string &bodyStr) = 0;
};

struct JsonBody : MessageBody
{
    JsonBody() = default;
    explicit JsonBody(const Json::Value &value);
    explicit JsonBody(Json::Value &&value);
    std::string BodySerialize() const;
    MessageBody::ptr BodyDeSerialize(const std::string &bodyStr);

    Json::Value m_bodyData;
};

struct ProtocolMessage
{
    MessageHeader m_header;
    MessageBody::ptr p_body;

    std::string Serialize();
    static ProtocolMessage DeSerialize(const MessageHeader &header, const std::string &bodyStr);
};