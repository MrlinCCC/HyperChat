cmake_minimum_required(VERSION 3.10)
project(HyperChatServer)

set(CMAKE_CXX_STANDARD 17)

#include asio.hpp
set(VCPKG_ROOT $ENV{VCPKG_ROOT})

find_path(ASIO_INCLUDE_DIR asio.hpp
  HINTS "${VCPKG_ROOT}/installed/x64-windows/include"
)

if(ASIO_INCLUDE_DIR)
  message(STATUS "Found asio include dir: ${ASIO_INCLUDE_DIR}")
  include_directories(${ASIO_INCLUDE_DIR})
else()
  message(FATAL_ERROR "asio not found")
endif()

include_directories(include)

add_executable(HyperChatServer src/main.cpp)