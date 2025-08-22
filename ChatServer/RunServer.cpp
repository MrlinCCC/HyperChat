#include <iostream>
#include <ChatServer.h>
#include "Logger.h"
#include <filesystem>
#include "ChatService.h"

void InitLogger()
{
	std::filesystem::path logFile = std::filesystem::path(PROJECT_LOG_ROOT) / "ChatServerLog.log";
	SET_LOGFILE(logFile.string());
	SET_LOGLEVEL(DEBUG);
}

int main(int argc, char* argv[])
{
	InitLogger();
	//if (argc < 2)
	//{
	//    LOG_ERROR("Usage: chatServer <port> [<threadNum>]");
	//    return -1;
	//}
	//unsigned short port;
	//size_t threadNum = 4;
	//try
	//{
	//    port = std::atoi(argv[1]);
	//}
	//catch (...)
	//{
	//    LOG_ERROR("Invalid port");
	//    return -1;
	//}

	//if (argc >= 3)
	//{
	//    threadNum = std::atoi(argv[2]);
	//    if (threadNum <= 0)
	//    {
	//        LOG_ERROR("Invalid thread num");
	//        return -1;
	//    }
	//}
	unsigned short port = 8888;
	size_t threadNum = 4;
	ChatServer server(port, threadNum);
	ChatService::GetInstance();
	server.Run();
	return 0;
}