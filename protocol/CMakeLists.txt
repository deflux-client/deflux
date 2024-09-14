cmake_minimum_required(VERSION 3.28)
project(deflux_proto)

set(CMAKE_CXX_STANDARD 20)

add_library(deflux_protocol INTERFACE include/deflux/protocol.hpp)
target_include_directories(deflux_protocol INTERFACE include)
target_link_libraries(deflux_protocol INTERFACE nlohmann_json::nlohmann_json)
