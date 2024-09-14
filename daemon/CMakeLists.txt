cmake_minimum_required(VERSION 3.28)
project(defluxd)

set(CMAKE_CXX_STANDARD 20)

add_executable(defluxd
        main.cpp
        io_pool.hpp
        io_pool.cpp
        tcp_server.hpp
        tcp_server.cpp
        tcp_connection.hpp
        tcp_connection.cpp)

target_link_libraries(defluxd PRIVATE spdlog::spdlog deflux_protocol)
target_include_directories(defluxd PRIVATE
        ../third-party/asio/asio/include)
