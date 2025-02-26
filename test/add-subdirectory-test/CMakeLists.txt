cmake_minimum_required(VERSION 3.1)

project(cxxopts-test)

add_subdirectory(../.. cxxopts EXCLUDE_FROM_ALL)

add_executable(library-test "../../src/example.cpp")
target_link_libraries(library-test cxxopts)
