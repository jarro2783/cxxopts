#include <regex>

namespace cxxopts
{
  std::basic_regex<char> option_matcher
    ("--([a-zA-Z][-_a-zA-Z]+)(=(.*))?|-([a-zA-Z]+)");

  std::basic_regex<char> option_specifier
    ("(([a-zA-Z]),)?([a-zA-Z][-_a-zA-Z]+)");
}
