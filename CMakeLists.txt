cmake_minimum_required(VERSION 3.28)
project(deflux)

set(CMAKE_CXX_STANDARD 20)

set(SPDLOG_USE_STD_FORMAT ON CACHE BOOL "Use std::format instead of fmt library." FORCE)

add_subdirectory(third-party/spdlog)
add_subdirectory(daemon)
