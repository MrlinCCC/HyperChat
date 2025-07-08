#pragma once
#include <iostream>
#include <json/json.h>
#include <unordered_map>
#include <functional>

enum BodyType : uint8_t
{
    JSON,
    //...
};

enum MessageType : uint8_t
{
    REGISTER,
    LOGIN,
    LOGOUT,
    ADD_FRIEND,
    ONE_CHAT,
    GROUP_CHAT,
    CREATE_GROUP,
    ADD_GROUP
};

enum class Status : uint8_t
{
    SUCCESS,

    INVALID_REQUEST,
    UNAUTHORIZED,
    FORBIDDEN,
    NOT_FOUND,
    RATE_LIMITED,
    UNSUPPORTED_BODYTYPE,

    REQUEST_TIMEOUT,
    SERVER_ERROR,
    BAD_GATEWAY,
    SERVICE_UNAVAILABLE,

    UNKNOWN_ERROR = 255
};

struct MessageHeader
{
    uint32_t m_magic = 0xA1B2C3D4;
    uint32_t m_bodyLength;
    uint32_t m_sessionId;
    BodyType m_bodyType;
    MessageType m_msgType;
    Status m_status;
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

class MessageBodyFactory
{
    using Creator = std::function<MessageBody::ptr()>;

public:
    static MessageBodyFactory &Instance()
    {
        static MessageBodyFactory factory;
        return factory;
    }
    void Register(BodyType type, Creator creator);
    MessageBody::ptr Create(BodyType type);

private:
    MessageBodyFactory();
    ~MessageBodyFactory() = default;
    MessageBodyFactory(const MessageBodyFactory &) = delete;
    MessageBodyFactory &operator=(const MessageBodyFactory &) = delete;
    MessageBodyFactory(const MessageBodyFactory &&) = delete;
    MessageBodyFactory &operator=(const MessageBodyFactory &&) = delete;

    std::unordered_map<BodyType, Creator> m_creator;
};