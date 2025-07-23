#include <iostream>
#include <ChatServer.h>
#include "Logger.h"
#include <filesystem>

void initLogger()
{
    std::filesystem::path logFile = std::filesystem::path(PROJECT_LOG_ROOT) / "TestLog_FileLogTest.log";
    SET_LOGFILE(logFile.string());
    SET_LOGLEVEL(DEBUG);
}

int main(int argc, char *argv[])
{
    initLogger();
    if (argc < 2)
    {
        LOG_ERROR("Usage: chatServer <port> [<threadNum>]");
        return -1;
    }
    unsigned short port = std::atoi(argv[1]);
    size_t threadNum = 4;

    if (argc >= 3)
    {
        threadNum = std::atoi(argv[2]);
        if (threadNum <= 0)
        {
            LOG_ERROR("Invalid thread num");
            return -1;
        }
    }
    ChatServer server(port, threadNum);
    server.Run();
    return 0;
}