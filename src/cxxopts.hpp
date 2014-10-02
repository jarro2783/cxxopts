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

#include <regex>
#include <set>
#include <map>
#include <exception>
#include <boost/any.hpp>
#include <sstream>

namespace cxxopts
{
  using boost::any;

  class Value
  {
    public:

    virtual void
    parse(const std::string& text) const = 0;

    virtual bool
    has_arg() const = 0;
  };

  namespace values
  {
    template <typename T>
    void
    parse_value(const std::string& text, T& value)
    {
      std::istringstream is(text);
      is >> value;
    }

    template <typename T>
    struct value_has_arg
    {
      static constexpr bool value = true;
    };

    template <>
    struct value_has_arg<bool>
    {
      static constexpr bool value = false;
    };

    template <typename T>
    class default_value : public Value
    {
      public:
      default_value()
      : m_result(std::make_shared<T>())
      , m_store(m_result.get())
      {
      }

      default_value(T* t)
      : m_store(t)
      {
      }

      void
      parse(const std::string& text) const
      {
        parse_value(text, *m_store);
      }

      bool
      has_arg() const
      {
        return value_has_arg<T>::value;
      }

      const T&
      get() const
      {
        if (m_store == nullptr)
        {
          return *m_result;
        }
        else
        {
          return *m_store;
        }
      }

      private:
      std::shared_ptr<T> m_result;
      T* m_store;
    };

  }

  template <typename T>
  std::shared_ptr<Value>
  value()
  {
    return std::make_shared<values::default_value<T>>();
  }

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
    , m_value(value)
    , m_count(0)
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
      return m_value->has_arg();
    }

    void
    parse(const std::string& text)
    {
      m_value->parse(text);
      ++m_count;
    }

    int
    count() const
    {
      return m_count;
    }

    template <typename T>
    const T&
    as() const
    {
      return dynamic_cast<const values::default_value<T>&>(*m_value).get();
    }

    private:
    std::string m_desc;
    std::shared_ptr<const Value> m_value;
    int m_count;
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
      auto iter = m_options.find(o);
      if (iter == m_options.end())
      {
        return 0;
      }

      return iter->second->count();
    }

    const OptionDetails&
    operator[](const std::string& option) const
    {
      auto iter = m_options.find(option);

      if (iter == m_options.end())
      {
        throw option_not_present_exception(option);
      }

      return *iter->second;
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


    std::map<std::string, std::shared_ptr<OptionDetails>> m_options;
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
        = ::cxxopts::value<bool>()
    );

    private:
    Options& m_options;
  };

}
