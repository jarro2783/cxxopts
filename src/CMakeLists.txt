add_executable(example main.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")

add_library(cxxopts SHARED cxxopts.cpp)

target_link_libraries(example cxxopts)
