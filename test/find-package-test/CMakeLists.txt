cmake_minimum_required(VERSION 3.1)

project(cxxopts-test)

set(CMAKE_CXX_STANDARD   11)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(cxxopts REQUIRED)

add_executable(library-test "../../src/example.cpp")
target_link_libraries(library-test cxxopts::cxxopts)
