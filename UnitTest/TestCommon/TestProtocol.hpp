#include <gtest/gtest.h>
#include "Protocol.h"

TEST(ProtocolCodecTest, RequestPackUnpackRoundTrip)
{
    auto req = std::make_shared<ProtocolRequest>();
    req->m_requestId = 12345;
    req->m_method = "GetUser";
    req->m_payload = "user=alice";

    std::string packed = ProtocolCodec::Instance().PackProtocolRequest(req);

    std::vector<char> buffer(packed.begin(), packed.end());

    auto unpacked = ProtocolCodec::Instance().UnPackProtocolRequest(buffer, buffer.size());
    ASSERT_EQ(unpacked.size(), 1);

    auto req2 = unpacked[0];
    EXPECT_EQ(req2->m_magic, MAGIC);
    EXPECT_EQ(req2->m_requestId, req->m_requestId);
    EXPECT_EQ(req2->m_method, req->m_method);
    EXPECT_EQ(req2->m_payload, req->m_payload);
}

TEST(ProtocolCodecTest, ResponsePackUnpackRoundTrip)
{
    auto resp = std::make_shared<ProtocolResponse>();
    resp->m_requestId = 54321;
    resp->m_pushType = "Notify";
    resp->m_status = BAD_REQUEST;
    resp->m_payload = "ok";

    std::string packed = ProtocolCodec::Instance().PackProtocolResponse(resp);
    std::vector<char> buffer(packed.begin(), packed.end());

    auto unpacked = ProtocolCodec::Instance().UnPackProtocolResponse(buffer, buffer.size());
    ASSERT_EQ(unpacked.size(), 1);

    auto resp2 = unpacked[0];
    EXPECT_EQ(resp2->m_magic, MAGIC);
    EXPECT_EQ(resp2->m_requestId, resp->m_requestId);
    EXPECT_EQ(resp2->m_pushType, resp->m_pushType);
    EXPECT_EQ(resp2->m_status, resp->m_status);
    EXPECT_EQ(resp2->m_payload, resp->m_payload);
}

TEST(ProtocolCodecTest, InvalidFirstMagicThenValidRequest)
{
    auto req1 = std::make_shared<ProtocolRequest>();
    req1->m_requestId = 111;
    req1->m_method = "Bad";
    req1->m_payload = "Payload1";
    std::string packed1 = ProtocolCodec::Instance().PackProtocolRequest(req1);

    auto req2 = std::make_shared<ProtocolRequest>();
    req2->m_requestId = 222;
    req2->m_method = "Good";
    req2->m_payload = "Payload2";
    std::string packed2 = ProtocolCodec::Instance().PackProtocolRequest(req2);

    packed1[0] = 0;
    packed1[1] = 0;
    packed1[2] = 0;
    packed1[3] = 0;

    std::string combined = packed1 + packed2;
    std::vector<char> buffer(combined.begin(), combined.end());

    auto unpacked = ProtocolCodec::Instance().UnPackProtocolRequest(buffer, buffer.size());

    ASSERT_EQ(unpacked.size(), 1);
    auto reqOut = unpacked[0];
    EXPECT_EQ(reqOut->m_requestId, req2->m_requestId);
    EXPECT_EQ(reqOut->m_method, req2->m_method);
    EXPECT_EQ(reqOut->m_payload, req2->m_payload);
}

TEST(ProtocolCodecTest, InvalidChecksumIsRejected)
{
    auto resp = std::make_shared<ProtocolResponse>();
    resp->m_requestId = 99;
    resp->m_pushType = "Push";
    resp->m_status = SUCCESS;
    resp->m_payload = "err";

    std::string packed = ProtocolCodec::Instance().PackProtocolResponse(resp);
    packed[packed.size() - 1] ^= 0xFF;

    std::vector<char> buffer(packed.begin(), packed.end());
    auto unpacked = ProtocolCodec::Instance().UnPackProtocolResponse(buffer, buffer.size());
    EXPECT_TRUE(unpacked.empty());
}
