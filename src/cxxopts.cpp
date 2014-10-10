/*

Copyright (c) 2014 Jarryd Beck

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

#include "cxxopts.hpp"

namespace cxxopts
{

  namespace
  {

    std::basic_regex<char> option_matcher
      ("--([[:alpha:]][-_[:alpha:]]+)(=(.*))?|-([a-zA-Z]+)");

    std::basic_regex<char> option_specifier
      ("(([a-zA-Z]),)?([a-zA-Z][-_a-zA-Z]+)");
  }

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

  m_options.add_option(s.str(), l.str(), desc, value);

  return *this;
}

void
Options::parse_option
(
  std::shared_ptr<OptionDetails> value,
  const std::string& name, 
  const std::string& arg
)
{
  value->parse(arg);
}

void
Options::checked_parse_arg
(
  int argc,
  char* argv[],
  int argPos,
  std::shared_ptr<OptionDetails> value,
  const std::string& name
)
{
  if (argPos >= argc)
  {
    throw missing_argument_exception(name);
  }

  parse_option(value, name, argv[argPos]);
}

void
Options::add_to_option(const std::string& option, const std::string& arg)
{
  auto iter = m_options.find(option);

  if (iter == m_options.end())
  {
    throw option_not_exists_exception(option);
  }

  parse_option(iter->second, option, arg);
}

bool
Options::consume_positional(std::string a)
{
  if (m_positional.size() > 0)
  {
    add_to_option(m_positional, a);
    return true;
  }
  else
  {
    return false;
  }
}

void
Options::parse_positional(std::string option)
{
  m_positional = std::move(option);
}

void
Options::parse(int& argc, char**& argv)
{
  int current = 1;

  int nextKeep = 1;

  while (current != argc)
  {
    std::match_results<const char*> result;
    std::regex_match(argv[current], result, option_matcher);

    if (result.empty())
    {
      //not a flag

      //if true is returned here then it was consumed, otherwise it is
      //ignored
      if (consume_positional(argv[current]))
      {
      }
      else
      {
        argv[nextKeep] = argv[current];
        ++nextKeep;
      }
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
          auto iter = m_options.find(name);

          if (iter == m_options.end())
          {
            throw option_not_exists_exception(name);
          }

          auto value = iter->second;

          //if no argument then just add it
          if (!value->has_arg())
          {
            parse_option(value, name);
          }
          else
          {
            //it must be the last argument
            if (i + 1 == s.size())
            {
              checked_parse_arg(argc, argv, current+1, value, name);
              ++current;
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
      
        auto iter = m_options.find(name);

        if (iter == m_options.end())
        {
          throw option_not_exists_exception(name);
        }

        auto opt = iter->second;

        //equals provided for long option?
        if (result[3].length() != 0)
        {
          //parse the option given

          //but if it doesn't take an argument, this is an error
          if (!opt->has_arg())
          {
            throw option_not_has_argument_exception(name, result[3]);
          }

          parse_option(opt, name, result[3]);
        }
        else
        {
          if (opt->has_arg())
          {
            //parse the next argument
            checked_parse_arg(argc, argv, current + 1, opt, name);

            ++current;
          }
          else
          {
            //parse with empty argument
            parse_option(opt, name);
          }
        }
      }

    }

    ++current;
  }

  argc = nextKeep;
}

void
Options::add_option
(
  const std::string& s, 
  const std::string& l, 
  const std::string& desc,
  std::shared_ptr<const Value> value
)
{
  auto option = std::make_shared<OptionDetails>(desc, value);

  if (s.size() > 0)
  {
    add_one_option(s, option);
  }

  if (l.size() > 0)
  {
    add_one_option(l, option);
  }

  //add the help details
}

void
Options::add_one_option
(
  const std::string& option,
  std::shared_ptr<OptionDetails> details
)
{
  auto in = m_options.insert(std::make_pair(option, details));

  if (!in.second)
  {
    throw option_exists_error(option);
  }
}

std::string
Options::help() const
{
  auto group = m_help.find("");
  if (group == m_help.end())
  {
    return "";
  }

  return "";
}

}
