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

add_executable(example example.cpp cxxopts.hpp)

# Visual Studio 12 2013 Update 5 or newer is required to fix in-class member
# initialization using std::make_shared<type>.
# https://connect.microsoft.com/VisualStudio/feedbackdetail/view/818825
if(MSVC AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 18.0.21005.1)
  message(SEND_ERROR "Visual Studio 12 2013 Update 5 or newer is required.")
endif()

target_link_libraries(example
  ${CXXOPTS_LINKER_LIBRARIES}
  )
target_compile_features(example PUBLIC
  cxx_auto_type
  )
target_include_directories(example PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
  )

include(WriteCompilerDetectionHeader)
write_compiler_detection_header(
  FILE cxxopts_compiler_detection.h
  PREFIX cxxopts
  COMPILERS GNU MSVC Clang
  FEATURES
    cxx_unicode_literals
    cxx_constexpr
    cxx_noexcept
)

install(FILES cxxopts.hpp ${CMAKE_CURRENT_BINARY_DIR}/cxxopts_compiler_detection.h DESTINATION include)
