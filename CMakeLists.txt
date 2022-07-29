cmake_minimum_required(VERSION 3.22)
project(bbserver2)

set(CMAKE_CXX_STANDARD 20)

add_executable(bbserver2 src/client.cpp src/common.cpp src/db_file_util.cpp src/main.cpp src/peer.cpp src/server.cpp src/string_helper.cpp)
