#include <iostream>
#include <asio.hpp>
#include <ChatClient.h>

int main(int argc, char *argv[])
{
    Host server("localhost", "12345");
    ChatClient client(server);
    client.connectServer();
    return 0;
}