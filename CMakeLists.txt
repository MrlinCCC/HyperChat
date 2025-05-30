cmake_minimum_required(VERSION 3.10)
project(HyperChatServer)

set(CMAKE_CXX_STANDARD 17)

#include asio.hpp
set(VCPKG_ROOT $ENV{VCPKG_ROOT})

add_executable(HyperChatServer app/RunServer.cpp src/ChatServer/ChatServerCore.cpp)
add_executable(HyperChatClient app/RunClient.cpp src/ChatClient/ChatClientCore.cpp)

find_path(ASIO_INCLUDE_DIR asio.hpp
  HINTS "${VCPKG_ROOT}/installed/x64-windows/include"
)

if(ASIO_INCLUDE_DIR)
  message(STATUS "Found asio include dir: ${ASIO_INCLUDE_DIR}")
  target_include_directories(HyperChatServer PUBLIC ${ASIO_INCLUDE_DIR})
  target_include_directories(HyperChatClient PUBLIC ${ASIO_INCLUDE_DIR})
else()
  message(FATAL_ERROR "asio not found")
endif()


target_include_directories(HyperChatServer PUBLIC include/ChatServer)
target_include_directories(HyperChatClient PUBLIC include/ChatClient)