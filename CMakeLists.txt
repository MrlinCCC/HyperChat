cmake_minimum_required(VERSION 3.10)
project(HyperChat)

set(CMAKE_CXX_STANDARD 17)

add_compile_definitions(PROJECT_ROOT="${CMAKE_SOURCE_DIR}")
add_compile_definitions(PROJECT_LOG_ROOT="${CMAKE_SOURCE_DIR}/log")

add_executable(HyperChatServer app/RunServer.cpp src/ChatServer/ChatServerCore.cpp)
add_executable(HyperChatClient app/RunClient.cpp src/ChatClient/ChatClientCore.cpp)

target_include_directories(HyperChatServer PUBLIC include/ChatServer)
target_include_directories(HyperChatClient PUBLIC include/ChatClient)

find_package(asio CONFIG REQUIRED)
target_link_libraries(HyperChatServer PRIVATE asio::asio)
target_link_libraries(HyperChatClient PRIVATE asio::asio)

add_subdirectory(include/Common)
add_subdirectory(UnitTest)
add_subdirectory(Benchmark)
