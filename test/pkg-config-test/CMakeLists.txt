cmake_minimum_required(VERSION 3.5...3.19)

project(cxxopts-pkg-config-test LANGUAGES CXX)

set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/prefix" CACHE PATH "" FORCE)
set(CXXOPTS_ENABLE_INSTALL ON CACHE BOOL "" FORCE)

if(TEST_INCLUDEDIR STREQUAL "absolute")
    set(CMAKE_INSTALL_INCLUDEDIR "${CMAKE_INSTALL_PREFIX}/custom/include" CACHE PATH "" FORCE)
    set(expected_includedir "${CMAKE_INSTALL_INCLUDEDIR}")
elseif(TEST_INCLUDEDIR STREQUAL "relative")
    set(CMAKE_INSTALL_INCLUDEDIR "custom/include" CACHE PATH "" FORCE)
    set(expected_includedir "\${prefix}/custom/include")
else()
    set(expected_includedir "\${prefix}/include")
endif()

add_subdirectory(../.. cxxopts EXCLUDE_FROM_ALL)

file(STRINGS "${CMAKE_CURRENT_BINARY_DIR}/cxxopts/cxxopts.pc" includedir
    REGEX "^includedir=")
if(NOT includedir STREQUAL "includedir=${expected_includedir}")
    message(FATAL_ERROR "Expected includedir=${expected_includedir}, got ${includedir}")
endif()
