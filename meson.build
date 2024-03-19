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

project(
  'cxxopts',
  'cpp',
  version : run_command(
    'meson/version.py', files('include/cxxopts.hpp'), capture : true, check : true).stdout().strip(),
  meson_version : '>= 0.64',
  license : 'MIT',
  default_options : ['cpp_std=c++11', 'warning_level=2'],
)

cpp = meson.get_compiler('cpp')

with_warnings = get_option('warnings').disable_auto_if(meson.is_subproject()).allowed()
if with_warnings
  add_project_arguments(
    cpp.get_supported_arguments(
      '-Wsuggest-override',
      '-Wshadow',
      '-Weffc++',
      '-Wsign-compare',
      '-Wshadow',
      '-Wwrite-strings',
      '-Wpointer-arith',
      '-Winit-self',
      '-Wconversion',
      '-Wno-sign-conversion',
    ),
    language : 'cpp',
  )
endif

install_headers('include/cxxopts.hpp')

dep_icu = dependency('icu-uc', required : get_option('icu'))
if dep_icu.found()
  add_project_arguments('-DCXXOPTS_USE_UNICODE', language : 'cpp')
endif

with_examples = get_option('examples').disable_auto_if(meson.is_subproject()).allowed()
if with_examples
  executable(
    'example',
    'src/example.cpp',
    include_directories : 'include',
    dependencies : dep_icu,
    override_options : ['cpp_std=c++17'],
  )
endif

with_tests = get_option('tests').disable_auto_if(meson.is_subproject()).allowed()
if with_tests
  subdir('test')
endif

extra_cflags = []
if dep_icu.found()
  extra_cflags += ['-DCXXOPTS_USE_UNICODE']
endif

dep_cxxopts = declare_dependency(
  include_directories : 'include',
  dependencies : dep_icu,
  compile_args : extra_cflags,
)

meson.override_dependency('cxxopts', dep_cxxopts)

pkg = import('pkgconfig')

pkg.generate(
  name : 'cxxopts',
  description : 'A header-only lightweight C++ command line option parser',
  url : 'https://github.com/jarro2783/cxxopts',
  extra_cflags : extra_cflags,
  requires : dep_icu,
  dataonly : not dep_icu.found(),
)
