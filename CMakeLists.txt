# Copyright (c) 2014 Jarryd Beck
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
cmake_minimum_required(VERSION 3.1)
project(cxxopts)

enable_testing()

file(STRINGS "${PROJECT_SOURCE_DIR}/include/cxxopts.hpp" cxxopts_version_defines
     REGEX "#define {CXXOPTS__VERSION_(MAJOR|MINOR|PATCH)")
foreach(ver ${cxxopts_version_defines})
    if(ver MATCHES "#define {CXXOPTS__VERSION_(MAJOR|MINOR|PATCH) +([^ ]+)$")
        set({CXXOPTS__VERSION_${CMAKE_MATCH_1} "${CMAKE_MATCH_2}" CACHE INTERNAL "")
    endif()
endforeach()
set(VERSION ${CXXOPTS__VERSION_MAJOR}.${CXXOPTS__VERSION_MINOR}.${CXXOPTS__VERSION_PATCH})

# set(VERSION "1.2.0")

option(CXXOPTS_BUILD_EXAMPLES "Set to ON to build examples" ON)
option(CXXOPTS_BUILD_TESTS "Set to ON to build tests" OFF)

# request c++11 without gnu extension for the whole project and enable more warnings
if (CXXOPTS_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD ${CXXOPTS_CXX_STANDARD})
else()
  set(CMAKE_CXX_STANDARD 11)
endif()

set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W2")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "[Cc]lang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wextra -Wshadow")
endif()

add_library(cxxopts INTERFACE)

# optionally, enable unicode support using the ICU library
set(CXXOPTS_USE_UNICODE_HELP FALSE CACHE BOOL "Use ICU Unicode library")
if(CXXOPTS_USE_UNICODE_HELP)
    find_package(PkgConfig)
    pkg_check_modules(ICU REQUIRED icu-uc)

    target_link_libraries(cxxopts INTERFACE ${ICU_LDFLAGS})
    target_compile_options(cxxopts INTERFACE ${ICU_CFLAGS})
    target_compile_definitions(cxxopts INTERFACE CXXOPTS_USE_UNICODE)
endif()

target_include_directories(cxxopts INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    )

include(CMakePackageConfigHelpers)
set(CXXOPTS_CMAKE_DIR "lib/cmake/cxxopts" CACHE STRING
  "Installation directory for cmake files, relative to ${CMAKE_INSTALL_PREFIX}.")
set(version_config "${PROJECT_BINARY_DIR}/cxxopts-config-version.cmake")
set(project_config "${PROJECT_BINARY_DIR}/cxxopts-config.cmake")
set(targets_export_name cxxopts-targets)

# Generate the version, config and target files into the build directory.
write_basic_package_version_file(
    ${version_config}
    VERSION ${VERSION}
    COMPATIBILITY AnyNewerVersion)
configure_package_config_file(
    ${PROJECT_SOURCE_DIR}/cxxopts-config.cmake.in
    ${project_config}
    INSTALL_DESTINATION ${CXXOPTS_CMAKE_DIR})
export(TARGETS cxxopts NAMESPACE cxxopts::
    FILE ${PROJECT_BINARY_DIR}/${targets_export_name}.cmake)

# Install version, config and target files.
install(
    FILES ${project_config} ${version_config}
    DESTINATION ${CXXOPTS_CMAKE_DIR})
install(EXPORT ${targets_export_name} DESTINATION ${CXXOPTS_CMAKE_DIR}
    NAMESPACE cxxopts::)

# Install the header file and export the target
install(TARGETS cxxopts EXPORT ${targets_export_name} DESTINATION lib)
install(FILES ${PROJECT_SOURCE_DIR}/include/cxxopts.hpp DESTINATION include)

add_subdirectory(src)
add_subdirectory(test)
