# Copyright © 2024 Dylan Baker
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

inc = include_directories('../include')

test(
  'link',
  executable(
    'link_test',
    'link_a.cpp', 'link_b.cpp',
    dependencies : dep_icu,
    include_directories : inc,
  )
)

test(
  'options',
  executable(
    'options_test',
    'main.cpp', 'options.cpp',
    dependencies : dep_icu,
    include_directories : inc,
  )
)

if cpp.get_id() == 'clang' and host_machine.system() == 'linux'
  executable(
    'fuzzer',
    'fuzz.cpp',
    cpp_args : ['-fsanitize=fuzzer'],
    link_args : ['-fsanitize=fuzzer'],
    dependencies : dep_icu,
    include_directories : inc,
  )
endif

# Meson can generate basic cmake-configs files, but not when targets are used,
# so these tests don't make sense