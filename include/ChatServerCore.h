#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <Asio.hpp>

namespace ChatServerCore
{

    class Client
    {
    public:
        Client(asio::ip::tcp::socket socket, const int userId);
        ~Client();
        void sendMessage(const std::string &message);
        void receiveMessage();
        int getSocket() const;
        std::string getUsername() const;

    private:
        asio::ip::tcp::socket m_socket;
        int m_userId;
    };

    class ChatRoom
    {
    public:
        ChatRoom();
        ~ChatRoom();
        void addClient(std::shared_ptr<Client> client);
        void removeClient(int clientSocket);
        void broadcastMessage(const std::string &message);

    private:
        std::vector<int> clients;
    };

    class ChatServer
    {
    public:
        ChatServer(int port);
        ~ChatServer();
        void startServer();
        void stopServer();

    private:
        asio::io_context m_ioContext;
        std::shared_ptr<asio::ip::tcp::acceptor> p_acceptor;
        std::vector<std::shared_ptr<Client>> m_clients;
        std::vector<std::shared_ptr<ChatRoom>> m_chatRooms;
        bool m_isRunning;
    };
}