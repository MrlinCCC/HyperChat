#include "ProtocolMessage.h"

JsonBody::JsonBody(const Json::Value &data) : m_bodyData(data) {}

JsonBody::JsonBody(Json::Value &&data) : m_bodyData(std::move(data)) {}

std::string JsonBody::BodySerialize() const
{
    Json::StreamWriterBuilder writer;
    writer["emitUTF8"] = true;
    return Json::writeString(writer, m_bodyData);
}

MessageBody::ptr JsonBody::BodyDeSerialize(const std::string &bodyStr)
{
    Json::CharReaderBuilder builder;
    std::string errs;
    std::unique_ptr<Json::CharReader> reader(builder.newCharReader());

    if (!reader->parse(bodyStr.c_str(), bodyStr.c_str() + bodyStr.size(), &m_bodyData, &errs))
        throw std::runtime_error("Json parse error: " + errs);

    return std::make_shared<JsonBody>(*this);
}

std::string ProtocolMessage::Serialize()
{
    try
    {
        std::string body_str = p_body->BodySerialize();
        m_header.m_bodyLength = static_cast<uint32_t>(body_str.size());
        std::string result(reinterpret_cast<const char *>(&m_header), sizeof(MessageHeader));
        result += body_str;
        return result;
    }
    catch (...)
    {
        throw std::runtime_error("Serialize json body fail!");
    }
}

ProtocolMessage ProtocolMessage::DeSerialize(const MessageHeader &header, const std::string &bodyStr)
{
    ProtocolMessage message;
    message.m_header = header;

    switch (header.m_bodyType)
    {
    case BodyType::JSON:
    {
        JsonBody body;
        message.p_body = body.BodyDeSerialize(bodyStr);
        break;
    }
    default:
        throw std::runtime_error("DeSerialize unsupported body type!");
    }
    return message;
}

MessageBodyFactory::MessageBodyFactory()
{
    Register(BodyType::JSON, []()
             { return std::make_shared<JsonBody>(); });
}

void MessageBodyFactory::Register(BodyType type, Creator creator)
{
    m_creator[BodyType::JSON] = std::move(creator);
}

MessageBody::ptr MessageBodyFactory::Create(BodyType type)
{
    auto it = m_creator.find(type);
    if (it != m_creator.end())
    {
        it->second();
    }
    return nullptr;
}