#!/usr/bin/env python
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

"""Parse the cxxopts.hpp header to get the version."""

import sys

def main():
    versions = [None, None, None]
    with open(sys.argv[1], 'r', encoding='ascii') as f:
        for line in f:
            if line.startswith('#define CXXOPTS__VERSION_'):
                ver = line.rstrip().rsplit(' ', 1)[-1]
                if 'MAJOR' in line:
                    versions[0] = ver
                elif 'MINOR' in line:
                    versions[1] = ver
                elif 'PATCH' in line:
                    versions[2] = ver
                if None not in versions:
                    break

    assert None not in versions, \
        "Did not find all of the expected version strings in cxxopts.hpp"
    print('.'.join(versions))


if __name__ == "__main__":
    main()
