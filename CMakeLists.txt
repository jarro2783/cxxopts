cmake_minimum_required(VERSION 3.10)
project(cxxoptspp VERSION 3.3.1 LANGUAGES CXX)

# C++ standard
seti(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include directory
include_directories(include)

# Find nlohmann/json
find_package(nlohmann_json 3.2.0 REQUIRED)

# Define library sources
set(CXXOPTSPP_SOURCES
    src/cxxoptspp.cpp
    src/dependency_engine.cpp
    src/type_system.cpp
    src/reversible_parser.cpp
)

# Create static library
add_library(cxxoptspp STATIC ${CXXOPTSPP_SOURCES})

# Create shared library
add_library(cxxoptspp_shared SHARED ${CXXOPTSPP_SOURCES})
set_target_properties(cxxoptspp_shared PROPERTIES OUTPUT_NAME cxxoptspp)

# Link nlohmann/json
target_link_libraries(cxxoptspp PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(cxxoptspp_shared PRIVATE nlohmann_json::nlohmann_json)

# Include cxxopts.hpp from original library
file(GLOB CXXOPTS_HEADER "include/cxxopts/*.hpp")

# Installation
install(TARGETS cxxoptspp cxxoptspp_shared
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES include/cxxoptspp.hpp DESTINATION include)
install(FILES ${CXXOPTS_HEADER} DESTINATION include/cxxopts)

# Example application
add_executable(example_enhanced src/example_enhanced.cpp)
target_link_libraries(example_enhanced PRIVATE cxxoptspp nlohmann_json::nlohmann_json)

# Tests
add_subdirectory(test)

# Export library
export(TARGETS cxxoptspp cxxoptspp_shared FILE cxxoptsppTargets.cmake)
configure_file(cxxoptsppConfig.cmake.in cxxoptsppConfig.cmake @ONLY)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/cxxoptsppConfig.cmake" 
        DESTINATION lib/cmake/cxxoptspp)
install(EXPORT cxxoptsppTargets DESTINATION lib/cmake/cxxoptspp)