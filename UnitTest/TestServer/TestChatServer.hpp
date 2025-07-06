#pragma once
#include "gtest/gtest.h"
#include "ChatServer.h"
#include "ChatService.h"

using asio::ip::tcp;

TEST(TestChatServer, AcceptsConnection)
{
    constexpr unsigned short testPort = 15000;
    ChatServer server(testPort);
    std::thread serverThread([&]()
                             { server.Run(); });

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

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

TEST(TestChatServer, ReceivesAndRespondsCorrectly)
{
    constexpr unsigned short testPort = 15001;
    ChatServer server(testPort);
    ChatService::GetInstance().SetExecuteServiceCb([](const ProtocolMessage &message)
                                                   { return message; });
    std::thread serverThread([&]()
                             { server.Run(); });

    std::this_thread::sleep_for(std::chrono::seconds(2));

    asio::io_context io_context;
    tcp::socket socket(io_context);
    socket.connect(tcp::endpoint(asio::ip::make_address("127.0.0.1"), testPort));
    auto p_session = std::make_shared<Session>(std::move(socket), [](std::shared_ptr<Session> session, const ProtocolMessage &message)
                                               {
            EXPECT_EQ(message.m_header.m_msgType, LOGIN);
            EXPECT_EQ(message.m_header.m_sessionId, 1);
            EXPECT_EQ(message.m_header.m_bodyType, JSON);
            Json::Value &json = std::dynamic_pointer_cast<JsonBody>(message.p_body)->m_bodyData;
            EXPECT_EQ(json["username"], "xdd");
            EXPECT_EQ(json["passwd"], "123"); });

    ProtocolMessage msg;
    msg.m_header.m_msgType = LOGIN;
    msg.m_header.m_sessionId = 1;
    msg.m_header.m_bodyType = JSON;
    Json::Value json;
    json["username"] = "xdd";
    json["passwd"] = "123";
    msg.p_body = std::make_shared<JsonBody>(json);
    p_session->AsyncReadMessage();
    p_session->AsyncWriteMessage(msg);
    std::thread clientThread([&]()
                             { io_context.run(); });
    std::this_thread::sleep_for(std::chrono::seconds(2));
    server.Shutdown();
    serverThread.join();
    clientThread.join();
    p_session->CloseSession();
}
