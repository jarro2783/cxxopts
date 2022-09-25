[![Build Status](https://travis-ci.org/jarro2783/cxxopts.svg?branch=master)](https://travis-ci.org/jarro2783/cxxopts)

# Release versions

Note that `master` is generally a work in progress, and you probably want to use a
tagged release version.

## Version 3 breaking changes

If you have used version 2, there are a couple of breaking changes in version 3
that you should be aware of. If you are new to `cxxopts` you can skip this
section.

The parser no longer modifies its arguments, so you can pass a const `argc` and
`argv` and expect them not to be changed.

The `ParseResult` object no longer depends on the parser. So it can be returned
from a scope outside the parser and still work. Now that the inputs are not
modified, `ParseResult` stores a list of the unmatched arguments. These are
retrieved like follows:

```cpp
auto result = options.parse(argc, argv);
result.unmatched(); // get the unmatched arguments
```

# Quick start

This is a lightweight C++ option parser library, supporting the standard GNU
style syntax for options.

Options can be given as:

    --long
    --long=argument
    --long argument
    -a
    -ab
    -abc argument

where c takes an argument, but a and b do not.

Additionally, anything after `--` will be parsed as a positional argument.

## Basics

```cpp
#include <cxxopts.hpp>
```

Create a `cxxopts::Options` instance.

```cpp
cxxopts::Options options("MyProgram", "One line description of MyProgram");
```

Then use `add_options`.

```cpp
options.add_options()
  ("d,debug", "Enable debugging") // a bool parameter
  ("i,integer", "Int param", cxxopts::value<int>())
  ("f,file", "File name", cxxopts::value<std::string>())
  ("v,verbose", "Verbose output", cxxopts::value<bool>()->default_value("false"))
  ;
```

Options are declared with a long and an optional short option. A description
must be provided. The third argument is the value, if omitted it is boolean.
Any type can be given as long as it can be parsed, with operator>>.

To parse the command line do:

```cpp
auto result = options.parse(argc, argv);
```

To retrieve an option use `result.count("option")` to get the number of times
it appeared, and

```cpp
result["opt"].as<type>()
```

to get its value. If "opt" doesn't exist, or isn't of the right type, then an
exception will be thrown.

Note that the result of `options.parse` should only be used as long as the
`options` object that created it is in scope.

## Unrecognised arguments

You can allow unrecognised arguments to be skipped. This applies to both
positional arguments that are not parsed into another option, and `--`
arguments that do not match an argument that you specify. This is done by
calling:

```cpp
options.allow_unrecognised_options();
```

and in the result object they are retrieved with:

```cpp
result.unmatched()
```

## Exceptions

Exceptional situations throw C++ exceptions. There are two types of
exceptions: errors defining the options, and errors when parsing a list of
arguments. All exceptions derive from `cxxopts::exceptions::exception`. Errors
defining options derive from `cxxopts::exceptions::specification` and errors
parsing arguments derive from `cxxopts::exceptions::parsing`.

All exceptions define a `what()` function to get a printable string
explaining the error.

## Help groups

Options can be placed into groups for the purposes of displaying help messages.
To place options in a group, pass the group as a string to `add_options`. Then,
when displaying the help, pass the groups that you would like displayed as a
vector to the `help` function.

## Positional Arguments

Positional arguments are those given without a preceding flag and can be used
alongside non-positional arguments. There may be multiple positional arguments,
and the final positional argument may be a container type to hold a list of all
remaining positionals.

To set up positional arguments, first declare the options, then configure a
set of those arguments as positional like:

```cpp
options.add_options()
  ("script", "The script file to execute", cxxopts::value<std::string>())
  ("server", "The server to execute on", cxxopts::value<std::string>())
  ("filenames", "The filename(s) to process", cxxopts::value<std::vector<std::string>>());

options.parse_positional({"script", "server", "filenames"});

// Parse options the usual way
options.parse(argc, argv);
```

For example, parsing the following arguments:
~~~
my_script.py my_server.com file1.txt file2.txt file3.txt
~~~
will result in parsed arguments like the following table:

| Field         | Value                                     |
| ------------- | ----------------------------------------- |
| `"script"`    | `"my_script.py"`                          |
| `"server"`    | `"my_server.com"`                         |
| `"filenames"` | `{"file1.txt", "file2.txt", "file3.txt"}` |

## Default and implicit values

An option can be declared with a default or an implicit value, or both.

A default value is the value that an option takes when it is not specified
on the command line. The following specifies a default value for an option:

```cpp
cxxopts::value<std::string>()->default_value("value")
```

An implicit value is the value that an option takes when it is given on the
command line without an argument. The following specifies an implicit value:

```cpp
cxxopts::value<std::string>()->implicit_value("implicit")
```

If an option had both, then not specifying it would give the value `"value"`,
writing it on the command line as `--option` would give the value `"implicit"`,
and writing `--option=another` would give it the value `"another"`.

Note that the default and implicit value is always stored as a string,
regardless of the type that you want to store it in. It will be parsed as
though it was given on the command line.

Default values are not counted by `Options::count`.

## Boolean values

Boolean options have a default implicit value of `"true"`, which can be
overridden. The effect is that writing `-o` by itself will set option `o` to
`true`. However, they can also be written with various strings using `=value`.
There is no way to disambiguate positional arguments from the value following
a boolean, so we have chosen that they will be positional arguments, and
therefore, `-o false` does not work.

## `std::vector<T>` values

Parsing of list of values in form of an `std::vector<T>` is also supported, as long as `T`
can be parsed. To separate single values in a list the definition `CXXOPTS_VECTOR_DELIMITER`
is used, which is ',' by default. Ensure that you use no whitespaces between values because
those would be interpreted as the next command line option. Example for a command line option
that can be parsed as a `std::vector<double>`:

~~~
--my_list=1,-2.1,3,4.5
~~~

## Options specified multiple times

The same option can be specified several times, with different arguments, which will all
be recorded in order of appearance. An example:

~~~
--use train --use bus --use ferry
~~~

this is supported through the use of a vector of value for the option:

~~~
options.add_options()
  ("use", "Usable means of transport", cxxopts::value<std::vector<std::string>>())
~~~

## Custom help

The string after the program name on the first line of the help can be
completely replaced by calling `options.custom_help`. Note that you might
also want to override the positional help by calling `options.positional_help`.


## Example

Putting all together:
```cpp
int main(int argc, char** argv)
{
    cxxopts::Options options("test", "A brief description");

    options.add_options()
        ("b,bar", "Param bar", cxxopts::value<std::string>())
        ("d,debug", "Enable debugging", cxxopts::value<bool>()->default_value("false"))
        ("f,foo", "Param foo", cxxopts::value<int>()->default_value("10"))
        ("h,help", "Print usage")
    ;

    auto result = options.parse(argc, argv);

    if (result.count("help"))
    {
      std::cout << options.help() << std::endl;
      exit(0);
    }
    bool debug = result["debug"].as<bool>();
    std::string bar;
    if (result.count("bar"))
      bar = result["bar"].as<std::string>();
    int foo = result["foo"].as<int>();

    return 0;
}
```

# Linking

This is a header only library.

# Requirements

The only build requirement is a C++ compiler that supports C++11 features such as:

* regex
* constexpr
* default constructors

GCC >= 4.9 or clang >= 3.1 with libc++ are known to work.

The following compilers are known not to work:

* MSVC 2013

# TODO list

* Allow unrecognised options.
