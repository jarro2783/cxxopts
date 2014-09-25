#include <regex>
#include <set>

namespace cxxopts
{
  extern std::basic_regex<char> option_matcher;

  extern std::basic_regex<char> option_specifier;

  class OptionAdder;

  class Options
  {
    public:

    void
    parse(int& argc, char**& argv);

    OptionAdder
    add_options();

    private:
    friend class OptionAdder;

    std::set<char32_t> m_short;
    std::set<std::string> m_long;
  };

  class OptionAdder
  {
    public:

    OptionAdder(Options& options)
    : m_options(options)
    {
    }

    OptionAdder&
    operator()
    ( 
      const std::string& opts, 
      const std::string& desc
    );

    private:
    Options& m_options;
  };
}
