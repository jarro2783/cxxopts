#ifndef CXXOPTS_EXCEPTIONS_HPP_
#define CXXOPTS_EXCEPTIONS_HPP_

#include "common.hpp"

#include <exception>
#include <string>            // for operator+, allocator, string, char_traits
#include <type_traits>       // for is_base_of
#include <utility>           // for move

namespace cxxopts
{
  namespace
  {
#ifdef _WIN32
    const std::string LQUOTE("\'");
    const std::string RQUOTE("\'");
#else
    const std::string LQUOTE("‘");
    const std::string RQUOTE("’");
#endif
  } // namespace


  class OptionException : public std::exception
  {
    public:
    explicit OptionException(std::string  message)
    : m_message(std::move(message))
    {
    }

    CXXOPTS_NODISCARD
    const char*
    what() const noexcept override
    {
      return m_message.c_str();
    }

    private:
    std::string m_message;
  };

  class OptionSpecException : public OptionException
  {
    public:

    explicit OptionSpecException(const std::string& message)
    : OptionException(message)
    {
    }
  };

  class OptionParseException : public OptionException
  {
    public:
    explicit OptionParseException(const std::string& message)
    : OptionException(message)
    {
    }
  };

  class option_exists_error : public OptionSpecException
  {
    public:
    explicit option_exists_error(const std::string& option)
    : OptionSpecException("Option " + LQUOTE + option + RQUOTE + " already exists")
    {
    }
  };

  class invalid_option_format_error : public OptionSpecException
  {
    public:
    explicit invalid_option_format_error(const std::string& format)
    : OptionSpecException("Invalid option format " + LQUOTE + format + RQUOTE)
    {
    }
  };

  class option_syntax_exception : public OptionParseException {
    public:
    explicit option_syntax_exception(const std::string& text)
    : OptionParseException("Argument " + LQUOTE + text + RQUOTE +
        " starts with a - but has incorrect syntax")
    {
    }
  };

  class option_not_exists_exception : public OptionParseException
  {
    public:
    explicit option_not_exists_exception(const std::string& option)
    : OptionParseException("Option " + LQUOTE + option + RQUOTE + " does not exist")
    {
    }
  };

  class missing_argument_exception : public OptionParseException
  {
    public:
    explicit missing_argument_exception(const std::string& option)
    : OptionParseException(
        "Option " + LQUOTE + option + RQUOTE + " is missing an argument"
      )
    {
    }
  };

  class option_requires_argument_exception : public OptionParseException
  {
    public:
    explicit option_requires_argument_exception(const std::string& option)
    : OptionParseException(
        "Option " + LQUOTE + option + RQUOTE + " requires an argument"
      )
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
        "Option " + LQUOTE + option + RQUOTE +
        " does not take an argument, but argument " +
        LQUOTE + arg + RQUOTE + " given"
      )
    {
    }
  };

  class option_not_present_exception : public OptionParseException
  {
    public:
    explicit option_not_present_exception(const std::string& option)
    : OptionParseException("Option " + LQUOTE + option + RQUOTE + " not present")
    {
    }
  };

  class option_has_no_value_exception : public OptionException
  {
    public:
    explicit option_has_no_value_exception(const std::string& option)
    : OptionException(
        !option.empty() ?
        ("Option " + LQUOTE + option + RQUOTE + " has no value") :
        "Option has no value")
    {
    }
  };

  class argument_incorrect_type : public OptionParseException
  {
    public:
    explicit argument_incorrect_type
    (
      const std::string& arg
    )
    : OptionParseException(
        "Argument " + LQUOTE + arg + RQUOTE + " failed to parse"
      )
    {
    }
  };

  class option_required_exception : public OptionParseException
  {
    public:
    explicit option_required_exception(const std::string& option)
    : OptionParseException(
        "Option " + LQUOTE + option + RQUOTE + " is required but not present"
      )
    {
    }
  };

  template <typename T>
  void throw_or_mimic(const std::string& text)
  {
    static_assert(std::is_base_of<std::exception, T>::value,
                  "throw_or_mimic only works on std::exception and "
                  "deriving classes");

#ifndef CXXOPTS_NO_EXCEPTIONS
    // If CXXOPTS_NO_EXCEPTIONS is not defined, just throw
    throw T{text};
#else
    // Otherwise manually instantiate the exception, print what() to stderr,
    // and exit
    T exception{text};
    std::cerr << exception.what() << std::endl;
    std::exit(EXIT_FAILURE);
#endif
  }
} // namespace cxxopts

#endif //CXXOPTS_EXCEPTIONS_HPP_
