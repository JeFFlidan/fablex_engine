cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(FablexEngine)

set(CMAKE_CXX_STANDARD 20)

include(cmake/common.cmake)

add_subdirectory(third_party)
add_subdirectory(src)