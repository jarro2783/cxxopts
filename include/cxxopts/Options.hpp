#ifndef CXXOPTS_OPTIONS_HPP_
#define CXXOPTS_OPTIONS_HPP_

#include "common.hpp"
#include "exceptions.hpp"
#include "values.hpp"
#include "Option.hpp"
#include "OptionParser.hpp"
#include "ParseResult.hpp"

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include <cctype>
#include <cstddef>
#include <algorithm>
#include <initializer_list>
#include <iterator>
#include <map>
#include <string>
#include <tuple>
#include <type_traits>


namespace cxxopts
{
  class Options;

  class OptionAdder
  {
  public:

    OptionAdder(Options& options, std::string group)
    : m_options(options), m_group(std::move(group))
    {
    }

    OptionAdder&
    operator()
    (
    const std::string& opts,
    const std::string& desc,
    const std::shared_ptr<const Value>& value
    = ::cxxopts::value<bool>(),
    std::string arg_help = ""
    );

  private:
    Options& m_options;
    std::string m_group;
  };


  class Options
  {
    public:

    explicit Options(std::string program, std::string help_string = "")
    : m_program(std::move(program))
    , m_help_string(toLocalString(std::move(help_string)))
    , m_custom_help("[OPTION...]")
    , m_positional_help("positional parameters")
    , m_show_positional(false)
    , m_allow_unrecognised(false)
    , m_width(76)
    , m_tab_expansion(false)
    , m_options(std::make_shared<OptionMap>())
    {
    }

    Options&
    positional_help(std::string help_text)
    {
      m_positional_help = std::move(help_text);
      return *this;
    }

    Options&
    custom_help(std::string help_text)
    {
      m_custom_help = std::move(help_text);
      return *this;
    }

    Options&
    show_positional_help()
    {
      m_show_positional = true;
      return *this;
    }

    Options&
    allow_unrecognised_options()
    {
      m_allow_unrecognised = true;
      return *this;
    }

    Options&
    set_width(size_t width)
    {
      m_width = width;
      return *this;
    }

    Options&
    set_tab_expansion(bool expansion=true)
    {
      m_tab_expansion = expansion;
      return *this;
    }

    ParseResult
    parse(int argc, const char* const* argv);

    OptionAdder
    add_options(std::string group = "");

    void
    add_options
    (
      const std::string& group,
      std::initializer_list<Option> options
    );

    void
    add_option
    (
      const std::string& group,
      const Option& option
    );

    void
    add_option
    (
      const std::string& group,
      const std::string& s,
      const std::string& l,
      std::string desc,
      const std::shared_ptr<const Value>& value,
      std::string arg_help
    );

    //parse positional arguments into the given option
    void
    parse_positional(std::string option);

    void
    parse_positional(std::vector<std::string> options);

    void
    parse_positional(std::initializer_list<std::string> options);

    template <typename Iterator>
    void
    parse_positional(Iterator begin, Iterator end) {
      parse_positional(std::vector<std::string>{begin, end});
    }

    std::string
    help(const std::vector<std::string>& groups = {}) const;

    std::vector<std::string>
    groups() const;

    const HelpGroupDetails&
    group_help(const std::string& group) const;

    const std::string& program() const
    {
      return m_program;
    }

    private:

    void
    add_one_option
    (
      const std::string& option,
      const std::shared_ptr<OptionDetails>& details
    );

    String
    help_one_group(const std::string& group) const;

    void
    generate_group_help
    (
      String& result,
      const std::vector<std::string>& groups
    ) const;

    void
    generate_all_groups_help(String& result) const;

    std::string m_program{};
    String m_help_string{};
    std::string m_custom_help{};
    std::string m_positional_help{};
    bool m_show_positional;
    bool m_allow_unrecognised;
    size_t m_width;
    bool m_tab_expansion;

    std::shared_ptr<OptionMap> m_options;
    std::vector<std::string> m_positional{};
    std::unordered_set<std::string> m_positional_set{};

    //mapping from groups to help options
    std::map<std::string, HelpGroupDetails> m_help{};
  };

  namespace
  {
    constexpr size_t OPTION_LONGEST = 30;
    constexpr size_t OPTION_DESC_GAP = 2;

    String
    format_option
    (
      const HelpOptionDetails& o
    )
    {
      const auto& s = o.s;
      const auto& l = o.l;

      String result = "  ";

      if (!s.empty())
      {
        result += "-" + toLocalString(s);
        if (!l.empty())
        {
          result += ",";
        }
      }
      else
      {
        result += "   ";
      }

      if (!l.empty())
      {
        result += " --" + toLocalString(l);
      }

      auto arg = !o.arg_help.empty() ? toLocalString(o.arg_help) : "arg";

      if (!o.is_boolean)
      {
        if (o.has_implicit)
        {
          result += " [=" + arg + "(=" + toLocalString(o.implicit_value) + ")]";
        }
        else
        {
          result += " " + arg;
        }
      }

      return result;
    }

    String
    format_description
    (
      const HelpOptionDetails& o,
      size_t start,
      size_t allowed,
      bool tab_expansion
    )
    {
      auto desc = o.desc;

      if (o.has_default && (!o.is_boolean || o.default_value != "false"))
      {
        if(!o.default_value.empty())
        {
          desc += toLocalString(" (default: " + o.default_value + ")");
        }
        else
        {
          desc += toLocalString(" (default: \"\")");
        }
      }

      String result;

      if (tab_expansion)
      {
        String desc2;
        auto size = size_t{ 0 };
        for (auto c = std::begin(desc); c != std::end(desc); ++c)
        {
          if (*c == '\n')
          {
            desc2 += *c;
            size = 0;
          }
          else if (*c == '\t')
          {
            auto skip = 8 - size % 8;
            stringAppend(desc2, skip, ' ');
            size += skip;
          }
          else
          {
            desc2 += *c;
            ++size;
          }
        }
        desc = desc2;
      }

      desc += " ";

      auto current = std::begin(desc);
      auto previous = current;
      auto startLine = current;
      auto lastSpace = current;

      auto size = size_t{};

      bool appendNewLine;
      bool onlyWhiteSpace = true;

      while (current != std::end(desc))
      {
        appendNewLine = false;

        if (std::isblank(*previous))
        {
          lastSpace = current;
        }

        if (!std::isblank(*current))
        {
          onlyWhiteSpace = false;
        }

        while (*current == '\n')
        {
          previous = current;
          ++current;
          appendNewLine = true;
        }

        if (!appendNewLine && size >= allowed)
        {
          if (lastSpace != startLine)
          {
            current = lastSpace;
            previous = current;
          }
          appendNewLine = true;
        }

        if (appendNewLine)
        {
          stringAppend(result, startLine, current);
          startLine = current;
          lastSpace = current;

          if (*previous != '\n')
          {
            stringAppend(result, "\n");
          }

          stringAppend(result, start, ' ');

          if (*previous != '\n')
          {
            stringAppend(result, lastSpace, current);
          }

          onlyWhiteSpace = true;
          size = 0;
        }

        previous = current;
        ++current;
        ++size;
      }

      //append whatever is left but ignore whitespace
      if (!onlyWhiteSpace)
      {
        stringAppend(result, startLine, previous);
      }

      return result;
    }
  } // namespace

  inline
  void
  Options::add_options
      (
          const std::string &group,
          std::initializer_list<Option> options
      )
  {
      OptionAdder option_adder(*this, group);
      for (const auto &option: options)
      {
          option_adder(option.opts_, option.desc_, option.value_, option.arg_help_);
      }
  }

  inline
  OptionAdder
  Options::add_options(std::string group)
  {
      return OptionAdder(*this, std::move(group));
  }

  inline
  OptionAdder&
  OptionAdder::operator()
      (
          const std::string& opts,
          const std::string& desc,
          const std::shared_ptr<const Value>& value,
          std::string arg_help
      )
  {
      std::string short_sw, long_sw;
      std::tie(short_sw, long_sw) = values::parser_tool::SplitSwitchDef(opts);

      if (!short_sw.length() && !long_sw.length())
      {
          throw_or_mimic<invalid_option_format_error>(opts);
      }
      else if (long_sw.length() == 1 && short_sw.length())
      {
          throw_or_mimic<invalid_option_format_error>(opts);
      }

      auto option_names = []
          (
              const std::string &short_,
              const std::string &long_
          )
      {
          if (long_.length() == 1)
          {
              return std::make_tuple(long_, short_);
          }
          return std::make_tuple(short_, long_);
      }(short_sw, long_sw);

      m_options.add_option
          (
              m_group,
              std::get<0>(option_names),
              std::get<1>(option_names),
              desc,
              value,
              std::move(arg_help)
          );

      return *this;
  }


  inline
  void
  Options::parse_positional(std::string option)
  {
      parse_positional(std::vector<std::string>{std::move(option)});
  }

  inline
  void
  Options::parse_positional(std::vector<std::string> options)
  {
      m_positional = std::move(options);

      m_positional_set.insert(m_positional.begin(), m_positional.end());
  }

  inline
  void
  Options::parse_positional(std::initializer_list<std::string> options)
  {
      parse_positional(std::vector<std::string>(options));
  }

  inline
  ParseResult
  Options::parse(int argc, const char* const* argv)
  {
      OptionParser parser(*m_options, m_positional, m_allow_unrecognised);

      return parser.parse(argc, argv);
  }


  inline
  void
  Options::add_option
      (
          const std::string& group,
          const Option& option
      )
  {
      add_options(group, {option});
  }

  inline
  void
  Options::add_option
      (
          const std::string& group,
          const std::string& s,
          const std::string& l,
          std::string desc,
          const std::shared_ptr<const Value>& value,
          std::string arg_help
      )
  {
      auto stringDesc = toLocalString(std::move(desc));
      auto option = std::make_shared<OptionDetails>(s, l, stringDesc, value);

      if (!s.empty())
      {
          add_one_option(s, option);
      }

      if (!l.empty())
      {
          add_one_option(l, option);
      }

      //add the help details
      auto& options = m_help[group];

      options.options.emplace_back(HelpOptionDetails{s, l, stringDesc,
                                                     value->has_default(), value->get_default_value(),
                                                     value->has_implicit(), value->get_implicit_value(),
                                                     std::move(arg_help),
                                                     value->is_container(),
                                                     value->is_boolean()});
  }

  inline
  void
  Options::add_one_option
      (
          const std::string& option,
          const std::shared_ptr<OptionDetails>& details
      )
  {
      auto in = m_options->emplace(option, details);

      if (!in.second)
      {
          throw_or_mimic<option_exists_error>(option);
      }
  }

  inline
  String
  Options::help_one_group(const std::string& g) const
  {
      using OptionHelp = std::vector<std::pair<String, String>>;

      auto group = m_help.find(g);
      if (group == m_help.end())
      {
          return "";
      }

      OptionHelp format;

      size_t longest = 0;

      String result;

      if (!g.empty())
      {
          result += toLocalString(" " + g + " options:\n");
      }

      for (const auto& o : group->second.options)
      {
          if (m_positional_set.find(o.l) != m_positional_set.end() &&
              !m_show_positional)
          {
              continue;
          }

          auto s = format_option(o);
          longest = (std::max)(longest, stringLength(s));
          format.push_back(std::make_pair(s, String()));
      }
      longest = (std::min)(longest, OPTION_LONGEST);

      //widest allowed description -- min 10 chars for helptext/line
      size_t allowed = 10;
      if (m_width > allowed + longest + OPTION_DESC_GAP)
      {
          allowed = m_width - longest - OPTION_DESC_GAP;
      }

      auto fiter = format.begin();
      for (const auto& o : group->second.options)
      {
          if (m_positional_set.find(o.l) != m_positional_set.end() &&
              !m_show_positional)
          {
              continue;
          }

          auto d = format_description(o, longest + OPTION_DESC_GAP, allowed, m_tab_expansion);

          result += fiter->first;
          if (stringLength(fiter->first) > longest)
          {
              result += '\n';
              result += toLocalString(std::string(longest + OPTION_DESC_GAP, ' '));
          }
          else
          {
              result += toLocalString(std::string(longest + OPTION_DESC_GAP -
                                                  stringLength(fiter->first),
                                                  ' '));
          }
          result += d;
          result += '\n';

          ++fiter;
      }

      return result;
  }

  inline
  void
  Options::generate_group_help
      (
          String& result,
          const std::vector<std::string>& print_groups
      ) const
  {
      for (size_t i = 0; i != print_groups.size(); ++i)
      {
          const String& group_help_text = help_one_group(print_groups[i]);
          if (empty(group_help_text))
          {
              continue;
          }
          result += group_help_text;
          if (i < print_groups.size() - 1)
          {
              result += '\n';
          }
      }
  }

  inline
  void
  Options::generate_all_groups_help(String& result) const
  {
      std::vector<std::string> all_groups;

      std::transform(
          m_help.begin(),
          m_help.end(),
          std::back_inserter(all_groups),
          [] (const std::map<std::string, HelpGroupDetails>::value_type& group)
          {
              return group.first;
          }
      );

      generate_group_help(result, all_groups);
  }

  inline
  std::string
  Options::help(const std::vector<std::string>& help_groups) const
  {
      String result = m_help_string + "\nUsage:\n  " +
                      toLocalString(m_program) + " " + toLocalString(m_custom_help);

      if (!m_positional.empty() && !m_positional_help.empty()) {
          result += " " + toLocalString(m_positional_help);
      }

      result += "\n\n";

      if (help_groups.empty())
      {
          generate_all_groups_help(result);
      }
      else
      {
          generate_group_help(result, help_groups);
      }

      return toUTF8String(result);
  }

  inline
  std::vector<std::string>
  Options::groups() const
  {
      std::vector<std::string> g;

      std::transform(
          m_help.begin(),
          m_help.end(),
          std::back_inserter(g),
          [] (const std::map<std::string, HelpGroupDetails>::value_type& pair)
          {
              return pair.first;
          }
      );

      return g;
  }

  inline
  const HelpGroupDetails&
  Options::group_help(const std::string& group) const
  {
      return m_help.at(group);
  }

} // namespace cxxopts

#endif //CXXOPTS_OPTIONS_HPP_
