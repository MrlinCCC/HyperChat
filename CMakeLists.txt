cmake_minimum_required(VERSION 3.10)
project(HyperChat)

set(CMAKE_CXX_STANDARD 17)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

add_compile_definitions(PROJECT_ROOT="${CMAKE_SOURCE_DIR}")
add_compile_definitions(PROJECT_LOG_ROOT="${CMAKE_SOURCE_DIR}/log")

find_package(fmt CONFIG REQUIRED)
find_package(asio CONFIG REQUIRED)

add_subdirectory(Common)
add_subdirectory(ChatServer)
add_subdirectory(ChatClient)
add_subdirectory(UnitTest)
add_subdirectory(Benchmark)