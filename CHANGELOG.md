# Changelog

This is the changelog for `cxxopts`, a C++11 library for parsing command line
options. The project adheres to semantic versioning.

## 2.0

### Changed

* `Options::parse` returns a ParseResult rather than storing the parse
  result internally.
* Options with default values now get counted as appearing once if they
  were not specified by the user.

### Added

* A new `ParseResult` object that is the immutable result of parsing. It
  responds to the same `count` and `operator[]` as `Options` of 1.x did.
* The function `ParseResult::arguments` returns a vector of the parsed
  arguments to iterate through in the order they were provided.
* The symbol `cxxopts::version` for the version of the library.
* Booleans can be specified with various strings and explicitly set false.

## 1.x

The 1.x series was the first major version of the library, with release numbers
starting to follow semantic versioning, after 0.x being unstable.  It never had
a changelog maintained for it. Releases mostly contained bug fixes, with the
occasional feature added.
