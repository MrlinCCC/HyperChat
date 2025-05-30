#include "ChatServerCore.h"

Client::Client(asio::ip::tcp::socket socket, const u_int clientId)
    : m_socket(std::move(socket)), m_clientId(clientId) {}

void Client::writeData(const std::string &data)
{
    asio::error_code ec;
    asio::write(m_socket, asio::buffer(data), ec);
    if (ec)
    {
        std::cerr << "ClientSocket " << m_clientId << " Write error: " << ec.message() << std::endl;
        return;
    }
}

std::string Client::readData()
{
    asio::error_code ec;
    std::size_t length = m_socket.read_some(asio::buffer(m_buffer), ec);
    std::cout << "ClientSocket " << m_clientId << " Read length: " << length << std::endl;
    if (ec)
    {
        std::cerr << "ClientSocket " << m_clientId << " Read error: " << ec.message() << std::endl;
        return "";
    }
    return std::string(m_buffer, length);
}

Client::~Client()
{
    closeSocket();
}

// ChatRoom::ChatRoom()
// {
// }
// ChatRoom::~ChatRoom()
// {
// }

ChatServer::ChatServer(u_int port)
{
    asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), port);
    p_acceptor = std::make_shared<asio::ip::tcp::acceptor>(m_ioContext, endpoint);
}
void ChatServer::runServer()
{
    m_isRunning = true;
    u_int user_count = 0;
    std::cout << "Hyper Chat server run on port " << p_acceptor->local_endpoint().port() << std::endl;
    while (m_isRunning)
    {
        asio::ip::tcp::socket socket(m_ioContext);
        asio::error_code ec;
        p_acceptor->accept(socket, ec);
        if (ec)
        {
            std::cerr << "Error accepting connection: " << ec.message() << std::endl;
            continue;
        }
        std::cout << "New client connected: " << socket.remote_endpoint().address().to_string() << ":" << socket.remote_endpoint().port() << std::endl;
        auto new_client = std::make_shared<Client>(std::move(socket), user_count++);
        {
            std::lock_guard<std::mutex> lock(m_clientsMutex);
            m_clients.push_back(new_client);
        }
        std::thread([this, new_client]()
                    {   if(!this->identifyClient(*new_client)) return;
                        this->listerMessage(*new_client); })
            .detach();
    }
}
void ChatServer::broadcastMessage(u_int sourceClientId, const std::string &message)
{
    std::lock_guard<std::mutex> lock(m_clientsMutex);
    for (auto &client : m_clients)
    {
        if (client->getClientId() == sourceClientId)
        {
            continue;
        }
        client->writeData(message);
        std::cout << "Broadcasting << message: " << message << std::endl;
    }
}

void ChatServer::listerMessage(Client &client)
{
    while (true)
    {
        std::string message = client.readData();
        if (message.empty())
        {
            std::cout << "Client " << client.getClientId() << " disconnected.\n";
            break;
        }
        broadcastMessage(client.getClientId(), message);
    }
}

bool ChatServer::identifyClient(Client &client)
{
    try
    {
        client.writeData("LOGIN:USERNAME");
        std::string username = client.readData();
        std::cout << "Client " << client.getClientId() << " identified as: " << username << std::endl;
        client.m_username = username;
        std::string token = this->generateToken(client);
        client.writeData(token);
        client.writeData("Welcome, " + username + "!");
        broadcastMessage(client.getClientId(), username + " has joined the chat.");
        return true;
    }
    catch (const std::exception &e)
    {

        std::cerr << "Identify Client " << client.getClientId() << " Fail!" << std::endl;
        return false;
    }
}

void ChatServer::stopServer()
{
    m_isRunning = false;
    p_acceptor->close();
    {
        std::lock_guard<std::mutex> lock(m_clientsMutex);
        m_clients.clear();
    }
    std::cout << "Chat server stopped." << std::endl;
}

std::string ChatServer::generateToken(const Client &client)
{
    return std::to_string(client.getClientId()) + "_" + client.m_username;
}

ChatServer::~ChatServer()
{
}
