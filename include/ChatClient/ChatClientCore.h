#pragma once
#include <iostream>
#include <Asio.hpp>
#define MAX_BUFFER_SIZE 1024

struct Host
{
    std::string hostName;
    std::string port;
    Host(const std::string &h, const std::string &p) : hostName(h), port(p) {}
};

class ChatClient
{
public:
    ChatClient(const Host &server);
    void connectServer();
    bool loginServer();
    bool sendMessage(const std::string &message);
    std::string receiveMessage();
    void closeConnection();

private:
    asio::io_context m_ioContext;
    Host m_server;
    asio::ip::tcp::socket m_serverSocket;
    char m_buffer[MAX_BUFFER_SIZE];
    bool m_isConnected;
    std::string m_token;
};

inline std::vector<std::string> split(const std::string &str, char delimiter)
{
    std::vector<std::string> tokens;
    size_t start = 0, end;
    while ((end = str.find(delimiter, start)) != std::string::npos)
    {
        tokens.emplace_back(str.substr(start, end - start));
        start = end + 1;
    }
    tokens.emplace_back(str.substr(start));
    return tokens;
}