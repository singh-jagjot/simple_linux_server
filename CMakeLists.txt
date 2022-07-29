cmake_minimum_required(VERSION 3.22)
project(bbserver2)

set(CMAKE_CXX_STANDARD 20)

add_executable(bbserver2 src/main.cpp src/server.cpp src/db_file_util.cpp src/common.cpp src/string_helper.cpp src/client.cpp)
