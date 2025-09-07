#include <gtest/gtest.h>
#include "ChatServer.h"

class ChatServerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_chatServer = std::make_unique<ChatServer>(m_port);
        m_curRequest = std::make_shared<ProtocolFrame>();
        m_chatServer->SetHandleProtocolFrame([&](const Session::Ptr &session, const std::shared_ptr<ProtocolFrame> &request)
                                             {
            handleCount++;
            EXPECT_EQ(request->m_header.m_requestId, m_curRequest->m_header.m_requestId);
            EXPECT_EQ(request->m_header.m_method, m_curRequest->m_header.m_method);
            EXPECT_EQ(request->m_payload,m_curRequest->m_payload);
            std::lock_guard<std::mutex> lock(mtx);
            cv.notify_one(); });
        m_serverThread = std::thread([this]()
                                     { m_chatServer->Run(); });

        std::this_thread::sleep_for(1s);

        m_ioc = std::make_unique<asio::io_context>();
        m_workGuard = std::make_unique<asio::executor_work_guard<asio::io_context::executor_type>>(
            asio::make_work_guard(*m_ioc));

        m_ioContextThread = std::thread([this]()
                                        { m_ioc->run(); });

        asio::ip::tcp::socket socket(*m_ioc);
        m_connection = std::make_shared<Connection>(std::move(socket));

        if (!m_connection->Connect("127.0.0.1", m_port))
        {
            throw std::runtime_error("Init connection failed!");
        }
    }

    void TearDown() override
    {
        std::this_thread::sleep_for(1s);
        if (m_chatServer)
        {
            m_chatServer->Shutdown();
            if (m_serverThread.joinable())
            {
                m_serverThread.join();
            }
        }

        m_connection->CloseConnection();
        if (m_workGuard)
        {
            m_workGuard.reset();
        }

        if (m_ioContextThread.joinable())
        {
            m_ioContextThread.join();
        }

        handleCount = 0;
    }

    unsigned short m_port = 8080;
    std::unique_ptr<ChatServer> m_chatServer;
    std::thread m_serverThread;
    std::thread m_ioContextThread;
    std::unique_ptr<asio::io_context> m_ioc;
    std::shared_ptr<Connection> m_connection;
    ProtocolCodec m_codec;
    std::unique_ptr<asio::executor_work_guard<asio::io_context::executor_type>> m_workGuard;
    std::shared_ptr<ProtocolFrame> m_curRequest;
    int handleCount = 0;
    std::mutex mtx;
    std::condition_variable cv;
};

TEST_F(ChatServerTest, NormalMessageReceiveTest)
{
    m_curRequest->m_header.m_requestId = 12345;
    m_curRequest->m_header.m_method = MethodType::REGISTER;
    m_curRequest->m_header.m_type = FrameType::REQUEST;
    m_curRequest->m_payload = "Test Payload";

    std::string packed = m_codec.PackProtocolFrame(m_curRequest);
    m_connection->AsyncWriteMessage(packed);

    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&]
            { return handleCount == 1; });
    cv.wait_for(lock, std::chrono::seconds(1), [&]
                { return handleCount >= 1; });
    EXPECT_EQ(handleCount, 1);
}

TEST_F(ChatServerTest, LargeMessageTest)
{

    std::string largePayload(4000, 'a');

    m_curRequest->m_header.m_requestId = 12345;
    m_curRequest->m_header.m_method = MethodType::REGISTER;
    m_curRequest->m_header.m_type = FrameType::REQUEST;
    m_curRequest->m_payload = largePayload;

    std::string packed = m_codec.PackProtocolFrame(m_curRequest);
    m_connection->AsyncWriteMessage(packed);
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait_for(lock, std::chrono::seconds(1), [&]
                { return handleCount >= 1; });
    EXPECT_EQ(handleCount, 1);
}

TEST_F(ChatServerTest, MessageSplitAcrossMultipleChunks)
{
    std::string largePayload(1024, 'a');

    m_curRequest->m_header.m_requestId = 12345;
    m_curRequest->m_header.m_method = MethodType::REGISTER;
    m_curRequest->m_header.m_type = FrameType::REQUEST;
    m_curRequest->m_payload = largePayload;

    std::string packed = m_codec.PackProtocolFrame(m_curRequest);

    size_t chunkSize = 256;
    for (size_t i = 0; i < packed.size(); i += chunkSize)
    {
        size_t endIdx = i + chunkSize > packed.size() ? packed.size() : i + chunkSize;
        std::string chunk(packed.begin() + i, packed.begin() + endIdx);
        m_connection->AsyncWriteMessage(chunk);
    }
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait_for(lock, std::chrono::seconds(1), [&]
                { return handleCount >= 1; });
    EXPECT_EQ(handleCount, 1);
}

TEST_F(ChatServerTest, PingPongTest)
{
    m_chatServer->ResetHandleProtocolFrame();
    std::thread t([this]()
                  {
        std::this_thread::sleep_for(std::chrono::milliseconds(SERVER_SESSION_TIMEOUT_MS));
        EXPECT_EQ(m_chatServer->GetSessionCount(), 1); });

    m_curRequest->m_header.m_requestId = 12345;
    m_curRequest->m_header.m_method = MethodType::REGISTER;
    m_curRequest->m_header.m_type = FrameType::PING;
    int receiveCount = 0;
    m_connection->SetMessageCallback([&](const Connection::Ptr &conn, std::size_t length)
                                     {
            auto frames=m_codec.UnPackProtocolFrame(conn->GetReadBuf(),length);
            ASSERT_EQ(frames.size(),1);
            const auto & f = frames[0];
            ASSERT_EQ(f->m_header.m_type,FrameType::PONG);
            receiveCount++;
            std::lock_guard<std::mutex> lock(mtx);
            cv.notify_one(); });
    m_connection->AsyncReadMessage();
    m_connection->AsyncWriteMessage(m_codec.PackProtocolFrame(m_curRequest));
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait_for(lock, std::chrono::seconds(1), [&]
                { return receiveCount >= 1; });
    t.join();
    EXPECT_EQ(receiveCount, 1);
}

TEST_F(ChatServerTest, HeartBeatResetTest)
{
    std::thread t([this]()
                  {
        std::this_thread::sleep_for(std::chrono::milliseconds(SERVER_SESSION_TIMEOUT_MS));
        EXPECT_EQ(m_chatServer->GetSessionCount(), 1); });

    std::this_thread::sleep_for(std::chrono::milliseconds(SESSION_ALIVE_CHECK_INTERNAL_MS / 2));

    m_curRequest->m_header.m_requestId = 12345;
    m_curRequest->m_header.m_method = MethodType::ONE_CHAT;
    m_curRequest->m_header.m_type = FrameType::REQUEST;
    m_connection->AsyncWriteMessage(m_codec.PackProtocolFrame(m_curRequest));

    t.join();
}

TEST_F(ChatServerTest, HeartBeatTimeoutTest)
{
    std::thread t([this]()
                  {
       std::this_thread::sleep_for(std::chrono::milliseconds(SERVER_SESSION_TIMEOUT_MS+SESSION_ALIVE_CHECK_INTERNAL_MS));
        EXPECT_EQ(m_chatServer->GetSessionCount(), 0); });

    t.join();
}