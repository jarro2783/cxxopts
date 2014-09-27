#include "cxxopts.hpp"

namespace cxxopts
{

  std::basic_regex<char> option_matcher
    ("--([[:alpha:]][-_[:alpha:]]+)(=(.*))?|-([a-zA-Z]+)");

  std::basic_regex<char> option_specifier
    ("(([a-zA-Z]),)?([a-zA-Z][-_a-zA-Z]+)");

OptionAdder
Options::add_options()
{
  return OptionAdder(*this);
}

OptionAdder&
OptionAdder::operator()
( 
  const std::string& opts, 
  const std::string& desc
)
{
  std::match_results<const char*> result;
  std::regex_match(opts.c_str(), result, option_specifier);

  if (result.empty())
  {
    throw invalid_option_format_error(opts);
  }

  const auto& s = result[2];
  const auto& l = result[3];

  auto option = std::make_shared<OptionDetails>(desc);

  if (s.length() != 0)
  {
    auto result = m_options.m_short.insert(std::make_pair(s.str()[0], option));

    if (!result.second)
    {
      throw option_exists_error(s.str());
    }
  }

  if (l.length() != 0)
  {
    auto result = m_options.m_long.insert(std::make_pair(l, option));

    if (!result.second)
    {
      throw option_exists_error(l.str());
    }
  }

  return *this;
}

}
