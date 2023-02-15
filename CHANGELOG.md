# Changelog

This is the changelog for `cxxopts`, a C++11 library for parsing command line
options. The project adheres to semantic versioning.

## 3.1.1

### Bug Fixes

* Fixed version number in header.

## 3.1

### Added

* Support for multiple long names for the same option (= multiple long aliases)
* Add a `program()` function to retrieve the program name.
* Added a .clang-format file.
* Added iterator and printing for a ParseResult.

### Changed

* Cleanup exception code, add cxxopts::exceptions namespace.

### Bug Fixes

* Fix `arguments()` having no key for options that only have a short name.

## 3.0

### Changed

* Only search for a C++ compiler in CMakeLists.txt.
* Allow for exceptions to be disabled.
* Fix duplicate default options when there is a short and long option.
* Add `CXXOPTS_NO_EXCEPTIONS` to disable exceptions.
* Fix char parsing for space and check for length.
* Change argument type in `Options::parse` from `char**` to `const char**`.
* Refactor parser to not change its arguments.
* `ParseResult` doesn't depend on a reference to the parser.
* Fixed several warnings and code quality issues.
* Improved formatting for help descriptions.
* Improve integer parsing.

### Added

* A list of unmatched arguments is available in `ParseResult`.
* Support single letter options with argument attached.
* Use <optional> if it is present.

### Bug Fixes

* Fix missing option name in exception.

## 2.2

### Changed

* Allow integers to have leading zeroes.
* Build the tests by default.
* Don't check for container when showing positional help.

### Added

* Iterator inputs to `parse_positional`.
* Throw an exception if the option in `parse_positional` doesn't exist.
* Parse a delimited list in a single argument for vector options.
* Add an option to disable implicit value on booleans.

### Bug Fixes

* Fix a warning about possible loss of data.
* Fix version numbering in CMakeLists.txt
* Remove unused declaration of the undefined `ParseResult::get_option`.
* Throw on invalid option syntax when beginning with a `-`.
* Throw in `as` when option wasn't present.
* Fix catching exceptions by reference.
* Fix out of bounds errors parsing integers.

## 2.1.1

### Bug Fixes

* Revert the change adding `const` type for `argv`, because most users expect
  to pass a non-const `argv` from `main`.

## 2.1

### Changed

* Options with implicit arguments now require the `--option=value` form if
  they are to be specified with an option. This is to remove the ambiguity
  when a positional argument could follow an option with an implicit value.
  For example, `--foo value`, where `foo` has an implicit value, will be
  parsed as `--foo=implicit` and a positional argument `value`.
* Boolean values are no longer special, but are just an option with a default
  and implicit value.

### Added

* Added support for `std::optional` as a storage type.
* Allow the help string to be customised.
* Use `const` for the type in the `argv` parameter, since the contents of the
  arguments is never modified.

### Bug Fixes

* Building against GCC 4.9 was broken due to overly strict shadow warnings.
* Fixed an ambiguous overload in the `parse_positional` function when an
  `initializer_list` was directly passed.
* Fixed precedence in the Boolean value regex.

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
