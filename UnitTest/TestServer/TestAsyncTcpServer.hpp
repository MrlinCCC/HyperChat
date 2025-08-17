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
		try
		{
			socket->connect(tcp::endpoint(asio::ip::make_address("127.0.0.1"), testPort));
		}
		catch (const std::system_error& e)
		{
			std::cout << "System error: " << e.what() << " Error code: " << e.code() << std::endl;
			throw;
		}
		sockets.push_back(std::move(socket));
	}

	std::this_thread::sleep_for(std::chrono::seconds(1));
	EXPECT_EQ(server.GetConnectionCount(), 10);
	for (auto& socket : sockets)
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
	int serverAcceptMsgCount = 0;
	server.SetOnMessage([&](const Connection::Ptr& connection, size_t length) {
		static std::vector<char> buffer;
		auto& data = connection->GetReadBuf();
		buffer.insert(buffer.end(), data.begin(), data.begin() + length);
		while (buffer.size() >= sizeof(uint32_t))
		{
			uint32_t messageLength = *reinterpret_cast<const uint32_t*>(buffer.data());
			if (buffer.size() >= sizeof(uint32_t) + messageLength)
			{
				std::string message(buffer.begin() + sizeof(uint32_t), buffer.begin() + sizeof(uint32_t) + messageLength);
				EXPECT_EQ(message, "Hello " + std::to_string(serverAcceptMsgCount));
				std::string response = "Server Response: " + message;
				uint32_t len = static_cast<uint32_t>(response.size());
				std::string msg(reinterpret_cast<const char*>(&len), sizeof(uint32_t));
				msg += response;
				serverAcceptMsgCount += 1;
				connection->AsyncWriteMessage(msg);
				buffer.erase(buffer.begin(), buffer.begin() + sizeof(uint32_t) + messageLength);
			}
			else
			{
				break;
			}
		}
		});

	std::thread serverThread([&]()
		{ server.Run(); });
	std::this_thread::sleep_for(std::chrono::milliseconds(200));

	asio::io_context io_context;
	auto lockGuard = asio::make_work_guard(io_context);
	tcp::socket socket(io_context);
	socket.connect(tcp::endpoint(asio::ip::make_address("127.0.0.1"), testPort));

	auto p_connection = std::make_shared<Connection>(std::move(socket));
	int clientAcceptMsgCount = 0;

	p_connection->SetMessageCallback([&](std::shared_ptr<Connection> connection, size_t length)
		{
			static std::vector<char> buffer;
			char* data = connection->GetReadBuf().data();
			buffer.insert(buffer.end(), data, data + length);

			while (buffer.size() >= sizeof(uint32_t))
			{
				uint32_t messageLength;
				std::memcpy(&messageLength, buffer.data(), sizeof(messageLength));
				if (buffer.size() >= sizeof(uint32_t) + messageLength)
				{
					std::string message(buffer.begin() + sizeof(uint32_t), buffer.begin() + sizeof(uint32_t) + messageLength);
					EXPECT_EQ(message, "Server Response: Hello " + std::to_string(clientAcceptMsgCount));
					buffer.erase(buffer.begin(), buffer.begin() + sizeof(uint32_t) + messageLength);
					++clientAcceptMsgCount;
				}
				else
				{
					break;
				}
			}
		});

	p_connection->AsyncReadMessage();

	std::thread clientThread([&]()
		{ io_context.run(); });

	int sendNum = 10;

	for (int i = 0; i < sendNum; ++i)
	{
		std::string message = "Hello " + std::to_string(i);
		uint32_t len = static_cast<uint32_t>(message.size());
		std::string buffer(reinterpret_cast<const char*>(&len), sizeof(len));
		buffer += message;
		p_connection->AsyncWriteMessage(buffer);
	}

	std::this_thread::sleep_for(std::chrono::seconds(1));

	p_connection->CloseConnection();

	lockGuard.reset();
	server.Shutdown();

	clientThread.join();
	serverThread.join();

	EXPECT_EQ(serverAcceptMsgCount, sendNum);
	EXPECT_EQ(clientAcceptMsgCount, sendNum);
}
