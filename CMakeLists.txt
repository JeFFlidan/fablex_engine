cmake_minimum_required(VERSION 3.28 FATAL_ERROR)

project(FablexEngine)

add_compile_options("/std:c++latest")

include(cmake/common.cmake)

add_subdirectory(third_party)
add_subdirectory(src)