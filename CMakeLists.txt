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

cmake_minimum_required(VERSION 2.6)
project(cxxopts)

set(VERSION "0.0.1")

set(CXXOPTS_LINKER_LIBRARIES "")

set(CXXOPTS_USE_UNICODE_HELP FALSE CACHE BOOL "Use ICU Unicode library")

if(CXXOPTS_USE_UNICODE_HELP)

  find_package(PkgConfig)

  pkg_check_modules(ICU REQUIRED icu-uc)

  set(CXXOPTS_LINKER_LIBRARIES "${ICU_LIBRARIES}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCXXOPTS_USE_UNICODE")

endif()

add_subdirectory(src)
