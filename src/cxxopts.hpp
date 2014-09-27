#include <regex>
#include <set>
#include <map>
#include <exception>
#include <boost/any.hpp>

namespace cxxopts
{
  using boost::any;

  class Value
  {
    public:

    virtual void
    parse(const std::string& text, any& result) const = 0;

    virtual bool
    has_arg() const = 0;
  };

  namespace values
  {
    class Boolean : public Value
    {
      void
      parse(const std::string& text, any& result) const
      {
      }

      bool
      has_arg() const
      {
        return false;
      }
    };
  }

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
    OptionDetails
    (
      const std::string& description,
      std::shared_ptr<const Value> value
    )
    : m_desc(description)
    , m_parser(value)
    {
    }

    const std::string& 
    description() const
    {
      return m_desc;
    }

    bool
    has_arg() const
    {
      return m_parser->has_arg();
    }

    private:
    boost::any m_value;
    std::string m_desc;
    std::shared_ptr<const Value> m_parser;
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

    std::map<std::string, std::shared_ptr<OptionDetails>> m_short;
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
      const std::string& desc,
      std::shared_ptr<const Value> value
        = std::make_shared<values::Boolean>()
    );

    private:
    Options& m_options;
  };
}
