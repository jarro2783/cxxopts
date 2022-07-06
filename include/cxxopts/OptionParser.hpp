#ifndef CXXOPTS_OPTIONPARSER_HPP_
#define CXXOPTS_OPTIONPARSER_HPP_

#include "common.hpp"
#include "KeyValue.hpp"
#include "Option.hpp"
#include "OptionValue.hpp"
#include "ParseResult.hpp"

#include <cstring>
#include <algorithm>
#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace cxxopts {

  using NameHashMap = std::unordered_map<std::string, size_t>;

  using OptionMap = std::unordered_map<std::string, std::shared_ptr<OptionDetails>>;
  using PositionalList = std::vector<std::string>;
  using PositionalListIterator = PositionalList::const_iterator;



  class OptionParser
  {
  public:
    OptionParser(const OptionMap& options, const PositionalList& positional, bool allow_unrecognised)
    : m_options(options)
    , m_positional(positional)
    , m_allow_unrecognised(allow_unrecognised)
    {
    }

    ParseResult
    parse(int argc, const char* const* argv);
    // Implemented along with ParseResult

    bool
    consume_positional(const std::string& a, PositionalListIterator& next);

    void
    checked_parse_arg
    (
    int argc,
    const char* const* argv,
    int& current,
    const std::shared_ptr<OptionDetails>& value,
    const std::string& name
    );

    void
    add_to_option(OptionMap::const_iterator iter, const std::string& option, const std::string& arg);

    void
    parse_option
    (
    const std::shared_ptr<OptionDetails>& value,
    const std::string& name,
    const std::string& arg = ""
    );

    void
    parse_default(const std::shared_ptr<OptionDetails>& details);

    void
    parse_no_value(const std::shared_ptr<OptionDetails>& details);

  private:

    void finalise_aliases();

    const OptionMap& m_options;
    const PositionalList& m_positional;

    std::vector<KeyValue> m_sequential{};
    std::vector<KeyValue> m_defaults{};
    bool m_allow_unrecognised;

    ParsedHashMap m_parsed{};
    NameHashMap m_keys{};
  };

  inline
  void
  OptionParser::finalise_aliases()
  {
    for (auto& option: m_options)
    {
      auto& detail = *option.second;
      auto hash = detail.hash();
      m_keys[detail.short_name()] = hash;
      m_keys[detail.long_name()] = hash;

      m_parsed.emplace(hash, OptionValue());
    }
  }

  inline
  void
  OptionParser::parse_default(const std::shared_ptr<OptionDetails>& details)
  {
    // TODO: remove the duplicate code here
    auto& store = m_parsed[details->hash()];
    store.parse_default(details);
    m_defaults.emplace_back(details->long_name(), details->value().get_default_value());
  }

  inline
  void
  OptionParser::parse_no_value(const std::shared_ptr<OptionDetails>& details)
  {
    auto& store = m_parsed[details->hash()];
    store.parse_no_value(details);
  }

  inline
  void
  OptionParser::parse_option
  (
  const std::shared_ptr<OptionDetails>& value,
  const std::string& /*name*/,
  const std::string& arg
  )
  {
    auto hash = value->hash();
    auto& result = m_parsed[hash];
    result.parse(value, arg);

    m_sequential.emplace_back(value->long_name(), arg);
  }

  inline
  void
  OptionParser::checked_parse_arg
  (
  int argc,
  const char* const* argv,
  int& current,
  const std::shared_ptr<OptionDetails>& value,
  const std::string& name
  )
  {
    if (current + 1 >= argc)
    {
      if (value->value().has_implicit())
      {
        parse_option(value, name, value->value().get_implicit_value());
      }
      else
      {
        throw_or_mimic<missing_argument_exception>(name);
      }
    }
    else
    {
      if (value->value().has_implicit())
      {
        parse_option(value, name, value->value().get_implicit_value());
      }
      else
      {
        parse_option(value, name, argv[current + 1]);
        ++current;
      }
    }
  }

  inline
  void
  OptionParser::add_to_option(OptionMap::const_iterator iter, const std::string& option, const std::string& arg)
  {
    parse_option(iter->second, option, arg);
  }

  inline
  bool
  OptionParser::consume_positional(const std::string& a, PositionalListIterator& next)
  {
    while (next != m_positional.end())
    {
      auto iter = m_options.find(*next);
      if (iter != m_options.end())
      {
        if (!iter->second->value().is_container())
        {
          auto& result = m_parsed[iter->second->hash()];
          if (result.count() == 0)
          {
            add_to_option(iter, *next, a);
            ++next;
            return true;
          }
          ++next;
          continue;
        }
        add_to_option(iter, *next, a);
        return true;
      }
      throw_or_mimic<option_not_exists_exception>(*next);
    }

    return false;
  }

  inline ParseResult
  OptionParser::parse(int argc, const char* const* argv)
  {
    int current = 1;
    bool consume_remaining = false;
    auto next_positional = m_positional.begin();

    std::vector<std::string> unmatched;

    while (current != argc)
    {
      if (strcmp(argv[current], "--") == 0)
      {
        consume_remaining = true;
        ++current;
        break;
      }
      bool matched = false;
      values::parser_tool::ArguDesc argu_desc =
      values::parser_tool::ParseArgument(argv[current], matched);

      if (!matched)
      {
        //not a flag

        // but if it starts with a `-`, then it's an error
        if (argv[current][0] == '-' && argv[current][1] != '\0') {
          if (!m_allow_unrecognised) {
            throw_or_mimic<option_syntax_exception>(argv[current]);
          }
        }

        //if true is returned here then it was consumed, otherwise it is
        //ignored
        if (consume_positional(argv[current], next_positional))
        {
        }
        else
        {
          unmatched.emplace_back(argv[current]);
        }
        //if we return from here then it was parsed successfully, so continue
      }
      else
      {
        //short or long option?
        if (argu_desc.grouping)
        {
          const std::string& s = argu_desc.arg_name;

          for (std::size_t i = 0; i != s.size(); ++i)
          {
            std::string name(1, s[i]);
            auto iter = m_options.find(name);

            if (iter == m_options.end())
            {
              if (m_allow_unrecognised)
              {
                unmatched.push_back(std::string("-") + s[i]);
                continue;
              }
              //error
              throw_or_mimic<option_not_exists_exception>(name);
            }

            auto value = iter->second;

            if (i + 1 == s.size())
            {
              //it must be the last argument
              checked_parse_arg(argc, argv, current, value, name);
            }
            else if (value->value().has_implicit())
            {
              parse_option(value, name, value->value().get_implicit_value());
            }
            else if (i + 1 < s.size())
            {
              std::string arg_value = s.substr(i + 1);
              parse_option(value, name, arg_value);
              break;
            }
            else
            {
              //error
              throw_or_mimic<option_requires_argument_exception>(name);
            }
          }
        }
        else if (argu_desc.arg_name.length() != 0)
        {
          const std::string& name = argu_desc.arg_name;

          auto iter = m_options.find(name);

          if (iter == m_options.end())
          {
            if (m_allow_unrecognised)
            {
              // keep unrecognised options in argument list, skip to next argument
              unmatched.emplace_back(argv[current]);
              ++current;
              continue;
            }
            //error
            throw_or_mimic<option_not_exists_exception>(name);
          }

          auto opt = iter->second;

          //equals provided for long option?
          if (argu_desc.set_value)
          {
            //parse the option given

            parse_option(opt, name, argu_desc.value);
          }
          else
          {
            //parse the next argument
            checked_parse_arg(argc, argv, current, opt, name);
          }
        }

      }

      ++current;
    }

    for (auto& opt : m_options)
    {
      auto& detail = opt.second;
      const auto& value = detail->value();

      auto& store = m_parsed[detail->hash()];

      if (value.has_default()) {
        if (!store.count() && !store.has_default()) {
          parse_default(detail);
        }
      }
      else {
        parse_no_value(detail);
      }
    }

    if (consume_remaining)
    {
      while (current < argc)
      {
        if (!consume_positional(argv[current], next_positional)) {
          break;
        }
        ++current;
      }

      //adjust argv for any that couldn't be swallowed
      while (current != argc) {
        unmatched.emplace_back(argv[current]);
        ++current;
      }
    }

    finalise_aliases();

    ParseResult parsed(std::move(m_keys), std::move(m_parsed), std::move(m_sequential), std::move(m_defaults), std::move(unmatched));
    return parsed;
  }

} // namespace cxxopts

#endif //CXXOPTS_OPTIONPARSER_HPP_
