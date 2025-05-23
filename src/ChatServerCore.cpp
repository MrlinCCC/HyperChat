#include "ChatServerCore.h"

namespace ChatServerCore
{

    Client::Client(asio::ip::tcp::socket socket, const int userId)
        : m_socket(std::move(socket)), m_userId(userId)
    {
    }
    Client::~Client()
    {
    }

    ChatRoom::ChatRoom()
    {
    }
    ChatRoom::~ChatRoom()
    {
    }

    ChatServer::ChatServer(int port)
    {
        asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), port);
        p_acceptor = std::make_shared<asio::ip::tcp::acceptor>(m_ioContext, endpoint);
    }
    void ChatServer::startServer()
    {
        m_isRunning = true;
        int user_count = 0;
        std::cout << "Hyper Chat server started on port " << p_acceptor->local_endpoint().port() << std::endl;
        while (m_isRunning)
        {
            asio::ip::tcp::socket socket(m_ioContext);
            asio::error_code ec;
            p_acceptor->accept(socket);
            if (ec)
            {
                std::cerr << "Error accepting connection: " << ec.message() << std::endl;
                continue;
            }
            else
            {
                std::cout << "New client connected: " << socket.remote_endpoint().address().to_string() << ":" << socket.remote_endpoint().port() << std::endl;
            }
            m_clients.push_back(std::make_shared<Client>(std::move(socket), user_count++));
        }
    }
    ChatServer::~ChatServer()
    {
        m_isRunning = false;
        if (p_acceptor)
        {
            p_acceptor->close();
        }
        if (m_clients.size() > 0)
        {
            m_clients.clear();
        }
    }
}
