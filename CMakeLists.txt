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
cmake_minimum_required(VERSION 3.1...3.19)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
include(cxxopts)
set("PROJECT_DESCRIPTION" "A header-only lightweight C++ command line option parser")
set("PROJECT_HOMEPAGE_URL" "https://github.com/jarro2783/cxxopts")

# Get the version of the library
cxxopts_getversion(VERSION)

project(cxxopts
    VERSION "${VERSION}"
    LANGUAGES CXX
)

# Must include after the project call due to GNUInstallDirs requiring a language be enabled (IE. CXX)
include(GNUInstallDirs)

# Determine whether this is a standalone project or included by other projects
set(CXXOPTS_STANDALONE_PROJECT OFF)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(CXXOPTS_STANDALONE_PROJECT ON)
endif()

# Establish the project options
option(CXXOPTS_BUILD_EXAMPLES "Set to ON to build examples" ${CXXOPTS_STANDALONE_PROJECT})
option(CXXOPTS_BUILD_TESTS "Set to ON to build tests" ${CXXOPTS_STANDALONE_PROJECT})
option(CXXOPTS_ENABLE_INSTALL "Generate the install target" ${CXXOPTS_STANDALONE_PROJECT})
option(CXXOPTS_ENABLE_WARNINGS "Add warnings to CMAKE_CXX_FLAGS" ${CXXOPTS_STANDALONE_PROJECT})
option(CXXOPTS_USE_UNICODE_HELP "Use ICU Unicode library" OFF)

if (CXXOPTS_STANDALONE_PROJECT)
    cxxopts_set_cxx_standard()
endif()

if (CXXOPTS_ENABLE_WARNINGS)
    cxxopts_enable_warnings()
endif()

add_library(cxxopts INTERFACE)
add_library(cxxopts::cxxopts ALIAS cxxopts)
add_subdirectory(include)

# Link against the ICU library when requested
if(CXXOPTS_USE_UNICODE_HELP)
    cxxopts_use_unicode()
endif()

# Install cxxopts when requested by the user
if (CXXOPTS_ENABLE_INSTALL)
    cxxopts_install_logic()
endif()

# Build examples when requested by the user
if (CXXOPTS_BUILD_EXAMPLES)
    add_subdirectory(src)
endif()

# Enable testing when requested by the user
if (CXXOPTS_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()
