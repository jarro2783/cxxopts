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
        result = true;
      }

      bool
      has_arg() const
      {
        return false;
      }
    };

    class String : public Value
    {
      void
      parse(const std::string& text, any& result) const
      {
        result = text;
      }

      bool
      has_arg() const
      {
        return true;
      }
    };
  }

  extern std::basic_regex<char> option_matcher;

  extern std::basic_regex<char> option_specifier;

  class MessageException
  ;

  class OptionException : public std::exception
  {
    public:
    OptionException(const std::string& message)
    : m_message(message)
    {
    }

    virtual const char*
    what() const noexcept
    {
      return m_message.c_str();
    }

    private:
    std::string m_message;
  };

  class OptionSpecException : public OptionException
  {
    public:

    OptionSpecException(const std::string& message)
    : OptionException(message)
    {
    }
  };

  class OptionParseException : public OptionException
  {
    public:
    OptionParseException(const std::string& message)
    : OptionException(message)
    {
    }
  };

  class option_exists_error : public OptionSpecException
  {
    public:
    option_exists_error(const std::string& option)
    : OptionSpecException(u8"Option ‘" + option + u8"’ already exists")
    {
    }
  };

  class invalid_option_format_error : public OptionSpecException
  {
    public:
    invalid_option_format_error(const std::string& format)
    : OptionSpecException(u8"Invalid option format ‘" + format + u8"’")
    {
    }
  };

  class option_not_exists_exception : public OptionParseException
  {
    public:
    option_not_exists_exception(const std::string& option)
    : OptionParseException(u8"Option ‘" + option + u8"’ does not exist")
    {
    }
  };

  class missing_argument_exception : public OptionParseException
  {
    public:
    missing_argument_exception(const std::string& option)
    : OptionParseException(u8"Option ‘" + option + u8"’ is missing an argument")
    {
    }
  };

  class option_requires_argument_exception : public OptionParseException
  {
    public:
    option_requires_argument_exception(const std::string& option)
    : OptionParseException(u8"Option ‘" + option + u8"’ requires an argument")
    {
    }
  };

  class option_not_has_argument_exception : public OptionParseException
  {
    public:
    option_not_has_argument_exception
    (
      const std::string& option, 
      const std::string& arg
    )
    : OptionParseException(
        u8"Option ‘" + option + u8"’ does not take an argument, but argument‘"
        + arg + "’ given")
    {
    }
  };

  class option_not_present_exception : public OptionParseException
  {
    public:
    option_not_present_exception(const std::string& option)
    : OptionParseException(u8"Option ‘" + option + u8"’ not present")
    {
    }
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

    void
    parse(const std::string& text, boost::any& arg)
    {
      m_parser->parse(text, arg);
    }

    private:
    std::string m_desc;
    std::shared_ptr<const Value> m_parser;
  };

  struct ParsedOption
  {
    boost::any value;
    int count;

    ParsedOption()
    : count(0)
    {
    }
  };

  class Options
  {
    public:

    void
    parse(int& argc, char**& argv);

    OptionAdder
    add_options();

    int
    count(const std::string& o) const
    {
      auto iter = m_parsed.find(o);

      if (iter == m_parsed.end())
      {
        return 0;
      }

      return iter->second.count;
    }

    const boost::any&
    operator[](const std::string& option) const
    {
      auto iter = m_parsed.find(option);

      if (iter == m_parsed.end())
      {
        throw option_not_present_exception(option);
      }

      return iter->second.value;
    }

    private:
    friend class OptionAdder;

    void
    parse_option
    (
      std::shared_ptr<OptionDetails> value,
      const std::string& name, 
      const std::string& arg = ""
    );
    
    void
    checked_parse_arg
    (
      int argc,
      char* argv[],
      int argPos,
      std::shared_ptr<OptionDetails> value,
      const std::string& name
    );


    std::map<std::string, std::shared_ptr<OptionDetails>> m_short;
    std::map<std::string, std::shared_ptr<OptionDetails>> m_long;

    std::map<std::string, ParsedOption> m_parsed;
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
