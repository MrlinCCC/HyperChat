add_executable(UnitTest RunTest.cpp)

find_package(GTest CONFIG REQUIRED)

target_link_libraries(UnitTest PRIVATE GTest::gtest GTest::gtest_main)
target_link_libraries(UnitTest PRIVATE Common HyperChatServerLib)

# enable_testing()

# add_test(NAME TestLog COMMAND UnitTest --gtest_filter=TestLog.*)