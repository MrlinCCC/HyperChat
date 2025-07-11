#include <gtest/gtest.h>
#include "TestCommon/TestLog.hpp"
#include "TestCommon/TestThreadPool.hpp"
#include "TestServer/TestChatServer.hpp"
#include <iostream>

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();
    std::cin.get();
    return result;
}