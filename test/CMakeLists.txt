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

add_executable(options_test main.cpp options.cpp)
target_link_libraries(options_test cxxopts)
add_test(options options_test)

# test if the targets are findable from the build directory
add_test(find-package-test ${CMAKE_CTEST_COMMAND}
    -C ${CMAKE_BUILD_TYPE}
    --build-and-test
    "${CMAKE_CURRENT_SOURCE_DIR}/find-package-test"
    "${CMAKE_CURRENT_BINARY_DIR}/find-package-test"
    --build-generator ${CMAKE_GENERATOR}
    --build-makeprogram ${CMAKE_MAKE_PROGRAM}
    --build-options
    "-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}"
    "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
    "-Dcxxopts_DIR=${PROJECT_BINARY_DIR}"
)

# test if the targets are findable when add_subdirectory is used
add_test(add-subdirectory-test ${CMAKE_CTEST_COMMAND}
    -C ${CMAKE_BUILD_TYPE}
    --build-and-test
    "${CMAKE_CURRENT_SOURCE_DIR}/add-subdirectory-test"
    "${CMAKE_CURRENT_BINARY_DIR}/add-subdirectory-test"
    --build-generator ${CMAKE_GENERATOR}
    --build-makeprogram ${CMAKE_MAKE_PROGRAM}
    --build-options
    "-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}"
    "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
)

add_executable(link_test link_a.cpp link_b.cpp)
target_link_libraries(link_test cxxopts)
