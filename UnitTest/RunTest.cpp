#include <gtest/gtest.h>
#include "TestCommon/TestLog.hpp"
#include "TestCommon/TestThreadPool.hpp"
#include "TestCommon/TestSerializer.hpp"
#include "TestCommon/TestProtocol.hpp"
#include "TestCommon/TestTimer.hpp"
#include "TestServer/TestAsyncTcpServer.hpp"
#include "TestServer/TestIoContextPool.hpp"
#include "TestServer/TestChatServer.hpp"
#include "TestServer/TestChatService.hpp"

#include <iostream>

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	int result = RUN_ALL_TESTS();
	std::cin.get();
	return result;
}