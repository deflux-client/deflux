cmake_minimum_required(VERSION 3.28)
project(defluxd)

set(CMAKE_CXX_STANDARD 20)

add_executable(defluxd main.cpp)

target_include_directories(defluxd PRIVATE
        ../third-party/asio/asio/include)
