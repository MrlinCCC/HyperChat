#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <Asio.hpp>
#define MAX_BUFFER_SIZE 1024

struct Client
{
    Client(asio::ip::tcp::socket socket, const u_int clientId);
    std::string readData();
    void writeData(const std::string &message);
    u_int getClientId() const { return m_clientId; }

    void closeSocket()
    {
        asio::error_code ec;
        m_socket.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
        if (ec)
        {
            std::cerr << "Shutdown socket error: " << ec.message() << std::endl;
        }
        m_socket.close(ec);
        if (ec)
        {
            std::cerr << "Close socket error: " << ec.message() << std::endl;
        }
    }

    ~Client();

    asio::ip::tcp::socket m_socket;
    u_int m_clientId;
    char m_buffer[MAX_BUFFER_SIZE];
    std::string m_lastRead;
    std::string m_username;
};

// class ChatSession
// {
// public:
// asio::ip::tcp::socket m_socket;
//     enum
//     {
//         max_length = 1024
//     };
//     char m_buffer[max_length];
// }

// class ChatRoom
// {
// public:
//     ChatRoom();
//     ~ChatRoom();
//     void addClient(std::shared_ptr<Client> client);
//     void removeClient(int clientSocket);
//     void broadcastMessage(const std::string &message);

// private:
//     std::vector<int> clients;
// };

class ChatServer
{
public:
    ChatServer(u_int port);
    ~ChatServer();
    void runServer();
    void broadcastMessage(u_int sourceClientId, const std::string &message);
    void listerMessage(Client &client);
    bool identifyClient(Client &client);
    void stopServer();
    std::string generateToken(const Client &client);

private:
    asio::io_context m_ioContext;
    std::shared_ptr<asio::ip::tcp::acceptor> p_acceptor;
    std::vector<std::shared_ptr<Client>> m_clients;
    std::mutex m_clientsMutex;
    // std::vector<std::shared_ptr<ChatRoom>> m_chatRooms;
    bool m_isRunning;
};