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

IF(BIICODE)
    include(biicode/cmake/tools)
    ADD_BIICODE_TARGETS()
    ACTIVATE_CPP11(INTERFACE ${BII_BLOCK_TARGET})
RETURN()

ENDIF()

cmake_minimum_required(VERSION 3.1)
project(cxxopts)

enable_testing()

set(VERSION "1.2.0")

option(CXXOPTS_BUILD_EXAMPLES "Set to ON to build examples" ON)
option(CXXOPTS_BUILD_TESTS "Set to ON to build tests" OFF)

set(CXXOPTS_LINKER_LIBRARIES "")
set(CXXOPTS_USE_UNICODE_HELP FALSE CACHE BOOL "Use ICU Unicode library")
if(CXXOPTS_USE_UNICODE_HELP)

  find_package(PkgConfig)

  pkg_check_modules(ICU REQUIRED icu-uc)

  set(CXXOPTS_LINKER_LIBRARIES "${ICU_LDFLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ICU_CFLAGS} -DCXXOPTS_USE_UNICODE")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wextra -Wshadow")

add_library(cxxopts INTERFACE)
target_sources(
  cxxopts INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}/include/cxxopts.hpp
  )
target_include_directories(
  cxxopts INTERFACE
  include/
  )
target_link_libraries(
  cxxopts
  INTERFACE ${CXXOPTS_LINKER_LIBRARIES}
  )

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/cxxopts.hpp DESTINATION include)

add_subdirectory(src)
add_subdirectory(test)
