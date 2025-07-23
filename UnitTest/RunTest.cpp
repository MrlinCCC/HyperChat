#include <gtest/gtest.h>
#include "TestCommon/TestLog.hpp"
#include "TestCommon/TestThreadPool.hpp"
#include "TestCommon/TestSerializer.hpp"
#include "TestServer/TestAsyncTcpServer.hpp"
#include "TestServer/TestIoContextPool.hpp"
#include <iostream>

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();
    std::cin.get();
    return result;
}