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
  const std::string& desc,
  std::shared_ptr<const Value> value
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

  auto option = std::make_shared<OptionDetails>(desc, value);

  if (s.length() != 0)
  {
    auto in = m_options.m_short.insert(std::make_pair(s.str(), option));

    if (!in.second)
    {
      throw option_exists_error(s.str());
    }
  }

  if (l.length() != 0)
  {
    auto in = m_options.m_long.insert(std::make_pair(l, option));

    if (!in.second)
    {
      throw option_exists_error(l.str());
    }
  }

  return *this;
}

void
Options::parse(int& argc, char**& argv)
{
  int current = 1;

  std::set<int> consumed;

  while (current != argc)
  {
    std::match_results<const char*> result;
    std::regex_match(argv[current], result, option_matcher);

    if (result.empty())
    {
      //handle empty

      //if we return from here then it was parsed successfully, so continue
    }
    else
    {
      //short or long option?
      if (result[4].length() != 0)
      {
        std::string s = result[4];

        for (int i = 0; i != s.size(); ++i)
        {
          std::string name(1, s[i]);
          auto iter = m_short.find(name);

          if (iter == m_short.end())
          {
            throw option_not_exists_exception(name);
            //argument not found
          }

          auto value = iter->second;

          //if no argument then just add it
          if (!value->has_arg())
          {
            auto& v = m_parsed[name];
            value->parse("", v);
          }
          else
          {
            //it must be the last argument
            if (i + 1 == s.size())
            {
            }
            else
            {
              //error
              throw option_requires_argument_exception(name);
            }
          }
        }
      }
      else if (result[1].length() != 0)
      {
        std::string name = result[1];
      
        auto iter = m_long.find(name);

        if (iter == m_long.end())
        {
          throw option_not_exists_exception(name);
        }

        auto opt = iter->second;

        //equals provided for long option?
        if (result[3].length() != 0)
        {
          //parse the option given
        }
        else
        {
          if (opt->has_arg())
          {
            //parse the next argument
          }
          else
          {
            //parse with empty argument
          }
        }
      }

    }

    ++current;
  }
}

}
