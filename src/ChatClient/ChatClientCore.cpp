#include "ChatClientCore.h"

ChatClient::ChatClient(const Host &server)
    : m_serverSocket(m_ioContext), m_server(server)
{
}

void ChatClient::connectServer()
{
    asio::ip::tcp::resolver resolver(m_ioContext);
    asio::error_code ec;
    asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(m_server.hostName, m_server.port, ec);
    if (ec)
    {
        std::cerr << "Resolve failed: " << ec.message() << std::endl;
        return;
    }
    asio::connect(m_serverSocket, endpoints, ec);
    if (ec)
    {
        std::cerr << "Connect Server error: " << ec.message() << std::endl;
        return;
    }
    m_isConnected = true;
    if (!this->loginServer())
    {
        closeConnection();
    }
    std::thread receiveThread([this]()
                              {
    while (m_isConnected)
    {
        std::string message=this->receiveMessage();
        if (message.empty()){
            std::cerr << "Server disconnected!" << std::endl;
            break;         
        }
            std::cout<< "Received message: " << message << std::endl;
    } });
    receiveThread.detach();
    while (true)
    {
        std::string message;
        std::cout << "Enter message: ";
        std::getline(std::cin, message);
        if (message == "exit")
            break;
        if (!this->sendMessage(message))
        {
            std::cerr << "Chat failed: send message fail." << std::endl;
            return;
        }
    }
    m_isConnected = false;
    m_serverSocket.close(ec);
    if (ec)
    {
        std::cerr << "Close socket error: " << ec.message() << std::endl;
    }
}

bool ChatClient::sendMessage(const std::string &message)
{
    asio::error_code ec;
    asio::write(m_serverSocket, asio::buffer(message), ec);
    if (ec)
    {
        std::cerr << "Send message error: " << ec.message() << std::endl;
        return false;
    }
    return true;
}

std::string ChatClient::receiveMessage()
{
    asio::error_code ec;
    size_t len = m_serverSocket.read_some(asio::buffer(m_buffer), ec);
    if (ec)
    {
        std::cerr << "Receive message error: " << ec.message() << std::endl;
        return "";
    }
    return std::string(m_buffer, len);
}

bool ChatClient::loginServer()
{
    std::string login_message = this->receiveMessage();
    if (login_message.empty())
    {
        std::cerr << "Login failed: No response from server." << std::endl;
        return false;
    }
    auto parsedServerCommand = split(login_message, ':');
    _ASSERT(parsedServerCommand.size() == 2);
    if (parsedServerCommand[0] != "LOGIN")
    {
        std::cerr << "Login failed: Invalid login header." << std::endl;
        return false;
    }
    if (parsedServerCommand[1] == "USERNAME")
    {
        std::string username;
        std::cout << "Please enter your username: ";
        std::getline(std::cin, username);
        if (!this->sendMessage(username))
        {
            std::cerr << "Login failed: send username fail." << std::endl;
            return false;
        }
    }
    else
    {
        std::cerr << "Valid login operator" << std::endl;
        return false;
    }
    m_token = this->receiveMessage();
    if (m_token.empty())
    {
        std::cerr << "Login failed: No token received." << std::endl;
        return false;
    }
    return true;
}

void ChatClient::closeConnection()
{
    m_isConnected = false;
    asio::error_code ec;
    m_serverSocket.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
    if (ec)
    {
        std::cerr << "Shutdown socket error: " << ec.message() << std::endl;
    }
    m_serverSocket.close(ec);
    if (ec)
    {
        std::cerr << "Close socket error: " << ec.message() << std::endl;
    }
    return;
}
