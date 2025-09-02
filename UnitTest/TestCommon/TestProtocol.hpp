#include <gtest/gtest.h>
#include "Protocol.h"

TEST(ProtocolCodecTest, RequestPackUnpack)
{
    auto req = std::make_shared<ProtocolFrame>();
    req->m_header.m_requestId = 12345;
    req->m_header.m_method = MethodType::LOGIN;
    req->m_header.m_type = FrameType::REQUEST;
    req->m_payload = "user=alice";

    ProtocolCodec codec;
    std::string packed = codec.PackProtocolFrame(req);

    std::string buffer(packed.begin(), packed.end());

    auto unpacked = codec.UnPackProtocolFrame(buffer.data(), buffer.size());
    ASSERT_EQ(unpacked.size(), 1);

    auto req2 = unpacked[0];
    EXPECT_EQ(req2->m_header.m_magic, Magic);
    EXPECT_EQ(req2->m_header.m_requestId, req->m_header.m_requestId);
    EXPECT_EQ(req2->m_header.m_method, req->m_header.m_method);
    EXPECT_EQ(req2->m_header.m_type, FrameType::REQUEST);
    EXPECT_EQ(req2->m_payload, req->m_payload);
}

TEST(ProtocolCodecTest, ResponsePackUnpack)
{
    auto resp = std::make_shared<ProtocolFrame>();
    resp->m_header.m_requestId = 54321;
    resp->m_header.m_method = MethodType::PUSH_MESSAGE;
    resp->m_header.m_type = FrameType::PUSH;
    resp->m_header.m_status = Status::BAD_REQUEST;
    resp->m_payload = "ok";

    ProtocolCodec codec;
    std::string packed = codec.PackProtocolFrame(resp);
    std::string buffer(packed.begin(), packed.end());

    auto unpacked = codec.UnPackProtocolFrame(buffer.data(), buffer.size());
    ASSERT_EQ(unpacked.size(), 1);

    auto resp2 = unpacked[0];
    EXPECT_EQ(resp2->m_header.m_magic, Magic);
    EXPECT_EQ(resp2->m_header.m_requestId, resp->m_header.m_requestId);
    EXPECT_EQ(resp2->m_header.m_method, resp->m_header.m_method);
    EXPECT_EQ(resp2->m_header.m_type, resp->m_header.m_type);
    EXPECT_EQ(resp2->m_header.m_status, resp->m_header.m_status);
    EXPECT_EQ(resp2->m_payload, resp->m_payload);
}

TEST(ProtocolCodecTest, InvalidFirstMagicThenValidRequest)
{
    auto req1 = std::make_shared<ProtocolFrame>();
    req1->m_header.m_magic = 111;
    req1->m_header.m_method = MethodType::ONE_CHAT;
    req1->m_header.m_type = FrameType::REQUEST;
    req1->m_payload = "Payload1";
    ProtocolCodec codec;
    std::string packed1 = codec.PackProtocolFrame(req1);

    auto req2 = std::make_shared<ProtocolFrame>();
    req2->m_header.m_requestId = 222;
    req2->m_header.m_method = MethodType::GROUP_CHAT;
    req1->m_header.m_type = FrameType::REQUEST;
    req2->m_payload = "Payload2";
    std::string packed2 = codec.PackProtocolFrame(req2);
    // modify magic
    packed1[0] = 0;
    packed1[1] = 0;
    packed1[2] = 0;
    packed1[3] = 0;

    std::string combined = packed1 + packed2;
    std::string buffer(combined.begin(), combined.end());

    auto unpacked = codec.UnPackProtocolFrame(buffer.data(), buffer.size());

    ASSERT_EQ(unpacked.size(), 1);
    auto reqOut = unpacked[0];
    EXPECT_EQ(reqOut->m_header.m_requestId, req2->m_header.m_requestId);
    EXPECT_EQ(reqOut->m_header.m_method, req2->m_header.m_method);
    EXPECT_EQ(reqOut->m_header.m_type, FrameType::REQUEST);
    EXPECT_EQ(reqOut->m_payload, req2->m_payload);
}

TEST(ProtocolCodecTest, InvalidChecksumIsRejected)
{
    auto resp = std::make_shared<ProtocolFrame>();
    resp->m_header.m_requestId = 99;
    resp->m_header.m_method = MethodType::LOGOUT;
    resp->m_header.m_type = FrameType::PUSH;
    resp->m_header.m_status = Status::SUCCESS;
    resp->m_payload = "err";

    ProtocolCodec codec;
    std::string packed = codec.PackProtocolFrame(resp);
    packed[packed.size() - 1] ^= 0xFF;

    std::string buffer(packed.begin(), packed.end());
    auto unpacked = codec.UnPackProtocolFrame(buffer.data(), buffer.size());
    EXPECT_TRUE(unpacked.empty());
}
