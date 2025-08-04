#pragma once
#include "gtest/gtest.h"
#include "AsyncTcpServer.h"

using asio::ip::tcp;

TEST(TestAsyncTcpServer, AcceptsConnection)
{
    constexpr unsigned short testPort = 15000;
    AsyncTcpServer server(testPort);
    std::thread serverThread([&]()
                             { server.Run(); });

    std::this_thread::sleep_for(std::chrono::seconds(1));

    asio::io_context io_context;
    tcp::socket socket(io_context);
    std::vector<std::unique_ptr<tcp::socket>> sockets;

    for (int i = 0; i < 10; ++i)
    {
        auto socket = std::make_unique<tcp::socket>(io_context);
        EXPECT_NO_THROW({
            socket->connect(tcp::endpoint(asio::ip::make_address("127.0.0.1"), testPort));
        });
        sockets.push_back(std::move(socket));
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
    ASSERT_EQ(server.GetConnectionCount(), 10);
    for (auto &socket : sockets)
    {
        if (socket->is_open())
        {
            socket->close();
        }
    }
    server.Shutdown();
    serverThread.join();
}

TEST(TestAsyncTcpServer, ReceivesAndRespondsCorrectly)
{
    constexpr unsigned short testPort = 15001;
    AsyncTcpServer server(testPort);
    server.SetHandleProtocolMessage([&](const Connection::Ptr &connection, const ProtocolRequest::Ptr &msg)
                                    { 
                                        EXPECT_EQ(msg->m_method, "LOGIN");
                                        EXPECT_EQ(msg->m_token, "XX");
                                        EXPECT_EQ(msg->m_payload, "TestRequestPayload");
                                        ProtocolResponse::Ptr responseMsg=std::make_shared<ProtocolResponse>();
                                        responseMsg->m_status = SUCCESS;
                                        responseMsg->m_payload = msg->m_payload;
                                        server.AsyncWriteMessage(connection, responseMsg); });
    std::thread serverThread([&]()
                             { server.Run(); });

    std::this_thread::sleep_for(std::chrono::seconds(1));

    asio::io_context io_context;
    tcp::socket socket(io_context);
    socket.connect(tcp::endpoint(asio::ip::make_address("127.0.0.1"), testPort));
    int acceptMsgCount = 0;
    ProtocolCodec protocolCodec;
    auto p_connection = std::make_shared<Connection>(std::move(socket));
    p_connection->SetMessageCallback([&](std::shared_ptr<Connection> connection, size_t length)
                                     {
            auto messages=protocolCodec.UnPackProtocolResponse(connection->GetReadBuf());
            for(auto message:messages){
                EXPECT_EQ(message->m_status, SUCCESS);
                EXPECT_EQ(message->m_payload, "123abc");
                acceptMsgCount+=1;
            } });

    p_connection->AsyncReadMessage();
    auto msg = std::make_shared<ProtocolRequest>();
    msg->m_method = "LOGIN";
    msg->m_token = "XX";
    msg->m_payload = "TestRequestPayload";
    for (int i = 0; i < 10; ++i)
    {
        p_connection->AsyncWriteMessage(protocolCodec.PackProtocolRequest(msg));
    }
    std::thread clientThread([&]()
                             { io_context.run(); });
    std::this_thread::sleep_for(std::chrono::seconds(1));
    ASSERT_EQ(acceptMsgCount, 10);
    server.Shutdown();
    serverThread.join();
    clientThread.join();
    p_connection->CloseConnection();
}
