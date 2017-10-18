# Changelog

This is the changelog for `cxxopts`, a C++11 library for parsing command line
options. The project adheres to semantic versioning.

## 2.0

### Changed

* `Options::parse` returns a ParseResult rather than storing the parse
  result internally.

### Added

* A new `ParseResult` object that is the immutable result of parsing. It
  responds to the same `count` and `operator[]` as `Options` of 1.x did.

## 1.x

The 1.x series was the first major version of the library, with release numbers
starting to follow semantic versioning, after 0.x being unstable.  It never had
a changelog maintained for it. Releases mostly contained bug fixes, with the
occasional feature added.
