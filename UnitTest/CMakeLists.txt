add_executable(UnitTest RunTest.cpp TestCommon/TestLog.hpp)

find_package(GTest CONFIG REQUIRED)

target_link_libraries(UnitTest PRIVATE GTest::gtest GTest::gtest_main)
target_link_libraries(UnitTest PRIVATE Logger)

# enable_testing()

# add_test(NAME TestLog COMMAND UnitTest --gtest_filter=TestLog.*)