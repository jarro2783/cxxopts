#include <regex>
#include <set>
#include <map>
#include <exception>

namespace cxxopts
{
  extern std::basic_regex<char> option_matcher;

  extern std::basic_regex<char> option_specifier;

  class OptionException : public std::exception
  {
  };

  class option_exists_error : public OptionException
  {
    public:
    option_exists_error(const std::string& option)
    {
      m_message = u8"Option ‘" + option + u8"’ already exists";
    }

    const char*
    what() const noexcept
    {
      return m_message.c_str();
    }

    private:
    std::string m_message;
  };

  class invalid_option_format_error : public OptionException
  {
    public:
    invalid_option_format_error(const std::string& format)
    {
      m_message = u8"Invalid option format ‘" + format + u8"’";
    }

    const char*
    what() const noexcept
    {
      return m_message.c_str();
    }

    private:
    std::string m_message;
  };

  class OptionAdder;

  class OptionDetails
  {
    public:
    OptionDetails(const std::string& description)
    : m_desc(description)
    {
    }

    private:
    std::string m_desc;
  };

  class Options
  {
    public:

    void
    parse(int& argc, char**& argv);

    OptionAdder
    add_options();

    private:
    friend class OptionAdder;

    std::map<char, std::shared_ptr<OptionDetails>> m_short;
    std::map<std::string, std::shared_ptr<OptionDetails>> m_long;
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
