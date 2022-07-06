#ifndef CXXOPTS_VALUES_HPP_
#define CXXOPTS_VALUES_HPP_

#include "common.hpp"
#include "exceptions.hpp"

#include <cstdint>
#include <map>
#include <limits>
#include <type_traits>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#if defined(__GNUC__) && !defined(__clang__)
#  if (__GNUC__ * 10 + __GNUC_MINOR__) < 49
#    define CXXOPTS_NO_REGEX true
#  endif
#endif

#ifndef CXXOPTS_NO_REGEX
#  include <regex>
#endif  // CXXOPTS_NO_REGEX

// Nonstandard before C++17, which is coincidentally what we also need for <optional>
#ifdef __has_include
#  if __has_include(<optional>)
#    include <optional>
#    ifdef __cpp_lib_optional
#      define CXXOPTS_HAS_OPTIONAL
#    endif
#  endif
#endif

#ifndef CXXOPTS_VECTOR_DELIMITER
#define CXXOPTS_VECTOR_DELIMITER ','
#endif

namespace cxxopts
{
#if defined(__GNUC__)
// GNU GCC with -Weffc++ will issue a warning regarding the upcoming class, we want to silence it:
// warning: base class 'class std::enable_shared_from_this<cxxopts::Value>' has accessible non-virtual destructor
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#pragma GCC diagnostic ignored "-Weffc++"
// This will be ignored under other compilers like LLVM clang.
#endif
class Value : public std::enable_shared_from_this<Value>
{
public:

    virtual ~Value() = default;

    virtual
    std::shared_ptr<Value>
    clone() const = 0;

    virtual void
    parse(const std::string& text) const = 0;

    virtual void
    parse() const = 0;

    virtual bool
    has_default() const = 0;

    virtual bool
    is_container() const = 0;

    virtual bool
    has_implicit() const = 0;

    virtual std::string
    get_default_value() const = 0;

    virtual std::string
    get_implicit_value() const = 0;

    virtual std::shared_ptr<Value>
    default_value(const std::string& value) = 0;

    virtual std::shared_ptr<Value>
    implicit_value(const std::string& value) = 0;

    virtual std::shared_ptr<Value>
    no_implicit_value() = 0;

    virtual bool
    is_boolean() const = 0;
};
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

  namespace values
  {
    namespace parser_tool
    {
      struct IntegerDesc
      {
        std::string negative = "";
        std::string base     = "";
        std::string value    = "";
      };
      struct ArguDesc {
        std::string arg_name  = "";
        bool        grouping  = false;
        bool        set_value = false;
        std::string value     = "";
      };
#ifdef CXXOPTS_NO_REGEX
      inline IntegerDesc SplitInteger(const std::string &text)
      {
        if (text.empty())
        {
          throw_or_mimic<argument_incorrect_type>(text);
        }
        IntegerDesc desc;
        const char *pdata = text.c_str();
        if (*pdata == '-')
        {
          pdata += 1;
          desc.negative = "-";
        }
        if (strncmp(pdata, "0x", 2) == 0)
        {
          pdata += 2;
          desc.base = "0x";
        }
        if (*pdata != '\0')
        {
          desc.value = std::string(pdata);
        }
        else
        {
          throw_or_mimic<argument_incorrect_type>(text);
        }
        return desc;
      }

      inline bool IsTrueText(const std::string &text)
      {
        const char *pdata = text.c_str();
        if (*pdata == 't' || *pdata == 'T')
        {
          pdata += 1;
          if (strncmp(pdata, "rue\0", 4) == 0)
          {
            return true;
          }
        }
        else if (strncmp(pdata, "1\0", 2) == 0)
        {
          return true;
        }
        return false;
      }

      inline bool IsFalseText(const std::string &text)
      {
        const char *pdata = text.c_str();
        if (*pdata == 'f' || *pdata == 'F')
        {
          pdata += 1;
          if (strncmp(pdata, "alse\0", 5) == 0)
          {
            return true;
          }
        }
        else if (strncmp(pdata, "0\0", 2) == 0)
        {
          return true;
        }
        return false;
      }

      inline std::pair<std::string, std::string> SplitSwitchDef(const std::string &text)
      {
        std::string short_sw, long_sw;
        const char *pdata = text.c_str();
        if (isalnum(*pdata) && *(pdata + 1) == ',') {
          short_sw = std::string(1, *pdata);
          pdata += 2;
        }
        while (*pdata == ' ') { pdata += 1; }
        if (isalnum(*pdata)) {
          const char *store = pdata;
          pdata += 1;
          while (isalnum(*pdata) || *pdata == '-' || *pdata == '_') {
            pdata += 1;
          }
          if (*pdata == '\0') {
            long_sw = std::string(store, pdata - store);
          } else {
            throw_or_mimic<invalid_option_format_error>(text);
          }
        }
        return std::pair<std::string, std::string>(short_sw, long_sw);
      }

      inline ArguDesc ParseArgument(const char *arg, bool &matched)
      {
        ArguDesc argu_desc;
        const char *pdata = arg;
        matched = false;
        if (strncmp(pdata, "--", 2) == 0)
        {
          pdata += 2;
          if (isalnum(*pdata))
          {
            argu_desc.arg_name.push_back(*pdata);
            pdata += 1;
            while (isalnum(*pdata) || *pdata == '-' || *pdata == '_')
            {
              argu_desc.arg_name.push_back(*pdata);
              pdata += 1;
            }
            if (argu_desc.arg_name.length() > 1)
            {
              if (*pdata == '=')
              {
                argu_desc.set_value = true;
                pdata += 1;
                if (*pdata != '\0')
                {
                  argu_desc.value = std::string(pdata);
                }
                matched = true;
              }
              else if (*pdata == '\0')
              {
                matched = true;
              }
            }
          }
        }
        else if (strncmp(pdata, "-", 1) == 0)
        {
          pdata += 1;
          argu_desc.grouping = true;
          while (isalnum(*pdata))
          {
            argu_desc.arg_name.push_back(*pdata);
            pdata += 1;
          }
          matched = !argu_desc.arg_name.empty() && *pdata == '\0';
        }
        return argu_desc;
      }

#else  // CXXOPTS_NO_REGEX

      namespace
      {

        std::basic_regex<char> integer_pattern
          ("(-)?(0x)?([0-9a-zA-Z]+)|((0x)?0)");
        std::basic_regex<char> truthy_pattern
          ("(t|T)(rue)?|1");
        std::basic_regex<char> falsy_pattern
          ("(f|F)(alse)?|0");

        std::basic_regex<char> option_matcher
          ("--([[:alnum:]][-_[:alnum:]]+)(=(.*))?|-([[:alnum:]]+)");
        std::basic_regex<char> option_specifier
          ("(([[:alnum:]]),)?[ ]*([[:alnum:]][-_[:alnum:]]*)?");

      } // namespace

      inline IntegerDesc SplitInteger(const std::string &text)
      {
        std::smatch match;
        std::regex_match(text, match, integer_pattern);

        if (match.length() == 0)
        {
          throw_or_mimic<argument_incorrect_type>(text);
        }

        IntegerDesc desc;
        desc.negative = match[1];
        desc.base = match[2];
        desc.value = match[3];

        if (match.length(4) > 0)
        {
          desc.base = match[5];
          desc.value = "0";
          return desc;
        }

        return desc;
      }

      inline bool IsTrueText(const std::string &text)
      {
        std::smatch result;
        std::regex_match(text, result, truthy_pattern);
        return !result.empty();
      }

      inline bool IsFalseText(const std::string &text)
      {
        std::smatch result;
        std::regex_match(text, result, falsy_pattern);
        return !result.empty();
      }

      inline std::pair<std::string, std::string> SplitSwitchDef(const std::string &text)
      {
        std::match_results<const char*> result;
        std::regex_match(text.c_str(), result, option_specifier);
        if (result.empty())
        {
          throw_or_mimic<invalid_option_format_error>(text);
        }

        const std::string& short_sw = result[2];
        const std::string& long_sw = result[3];

        return std::pair<std::string, std::string>(short_sw, long_sw);
      }

      inline ArguDesc ParseArgument(const char *arg, bool &matched)
      {
        std::match_results<const char*> result;
        std::regex_match(arg, result, option_matcher);
        matched = !result.empty();

        ArguDesc argu_desc;
        if (matched) {
          argu_desc.arg_name = result[1].str();
          argu_desc.set_value = result[2].length() > 0;
          argu_desc.value = result[3].str();
          if (result[4].length() > 0)
          {
            argu_desc.grouping = true;
            argu_desc.arg_name = result[4].str();
          }
        }

        return argu_desc;
      }

#endif  // CXXOPTS_NO_REGEX
#undef CXXOPTS_NO_REGEX
  }

    namespace detail
    {
      template <typename T, bool B>
      struct SignedCheck;

      template <typename T>
      struct SignedCheck<T, true>
      {
        template <typename U>
        void
        operator()(bool negative, U u, const std::string& text)
        {
          if (negative)
          {
            if (u > static_cast<U>((std::numeric_limits<T>::min)()))
            {
              throw_or_mimic<argument_incorrect_type>(text);
            }
          }
          else
          {
            if (u > static_cast<U>((std::numeric_limits<T>::max)()))
            {
              throw_or_mimic<argument_incorrect_type>(text);
            }
          }
        }
      };

      template <typename T>
      struct SignedCheck<T, false>
      {
        template <typename U>
        void
        operator()(bool, U, const std::string&) const {}
      };

      template <typename T, typename U>
      void
      check_signed_range(bool negative, U value, const std::string& text)
      {
        SignedCheck<T, std::numeric_limits<T>::is_signed>()(negative, value, text);
      }
    } // namespace detail

    template <typename R, typename T>
    void
    checked_negate(R& r, T&& t, const std::string&, std::true_type)
    {
      // if we got to here, then `t` is a positive number that fits into
      // `R`. So to avoid MSVC C4146, we first cast it to `R`.
      // See https://github.com/jarro2783/cxxopts/issues/62 for more details.
      r = static_cast<R>(-static_cast<R>(t-1)-1);
    }

    template <typename R, typename T>
    void
    checked_negate(R&, T&&, const std::string& text, std::false_type)
    {
      throw_or_mimic<argument_incorrect_type>(text);
    }

    template <typename T>
    void
    integer_parser(const std::string& text, T& value)
    {
      parser_tool::IntegerDesc int_desc = parser_tool::SplitInteger(text);

      using US = typename std::make_unsigned<T>::type;
      constexpr bool is_signed = std::numeric_limits<T>::is_signed;

      const bool          negative    = int_desc.negative.length() > 0;
      const uint8_t       base        = int_desc.base.length() > 0 ? 16 : 10;
      const std::string & value_match = int_desc.value;

      US result = 0;

      for (char ch : value_match)
      {
        US digit = 0;

        if (ch >= '0' && ch <= '9')
        {
          digit = static_cast<US>(ch - '0');
        }
        else if (base == 16 && ch >= 'a' && ch <= 'f')
        {
          digit = static_cast<US>(ch - 'a' + 10);
        }
        else if (base == 16 && ch >= 'A' && ch <= 'F')
        {
          digit = static_cast<US>(ch - 'A' + 10);
        }
        else
        {
          throw_or_mimic<argument_incorrect_type>(text);
        }

        const US next = static_cast<US>(result * base + digit);
        if (result > next)
        {
          throw_or_mimic<argument_incorrect_type>(text);
        }

        result = next;
      }

      detail::check_signed_range<T>(negative, result, text);

      if (negative)
      {
        checked_negate<T>(value, result, text, std::integral_constant<bool, is_signed>());
      }
      else
      {
        value = static_cast<T>(result);
      }
    }

    template <typename T>
    void stringstream_parser(const std::string& text, T& value)
    {
      std::stringstream in(text);
      in >> value;
      if (!in) {
        throw_or_mimic<argument_incorrect_type>(text);
      }
    }

    template <typename T,
             typename std::enable_if<std::is_integral<T>::value>::type* = nullptr
             >
    void parse_value(const std::string& text, T& value)
    {
        integer_parser(text, value);
    }

    inline
    void
    parse_value(const std::string& text, bool& value)
    {
      if (parser_tool::IsTrueText(text))
      {
        value = true;
        return;
      }

      if (parser_tool::IsFalseText(text))
      {
        value = false;
        return;
      }

      throw_or_mimic<argument_incorrect_type>(text);
    }

    inline
    void
    parse_value(const std::string& text, std::string& value)
    {
      value = text;
    }

    // The fallback parser. It uses the stringstream parser to parse all types
    // that have not been overloaded explicitly.  It has to be placed in the
    // source code before all other more specialized templates.
    template <typename T,
             typename std::enable_if<!std::is_integral<T>::value>::type* = nullptr
             >
    void
    parse_value(const std::string& text, T& value) {
      stringstream_parser(text, value);
    }

    template <typename T>
    void
    parse_value(const std::string& text, std::vector<T>& value)
    {
      if (text.empty()) {
        T v;
        parse_value(text, v);
        value.emplace_back(std::move(v));
        return;
      }
      std::stringstream in(text);
      std::string token;
      while(!in.eof() && std::getline(in, token, CXXOPTS_VECTOR_DELIMITER)) {
        T v;
        parse_value(token, v);
        value.emplace_back(std::move(v));
      }
    }

#ifdef CXXOPTS_HAS_OPTIONAL
    template <typename T>
    void
    parse_value(const std::string& text, std::optional<T>& value)
    {
      T result;
      parse_value(text, result);
      value = std::move(result);
    }
#endif

    inline
    void parse_value(const std::string& text, char& c)
    {
      if (text.length() != 1)
      {
        throw_or_mimic<argument_incorrect_type>(text);
      }

      c = text[0];
    }

    template <typename T>
    struct type_is_container
    {
      static constexpr bool value = false;
    };

    template <typename T>
    struct type_is_container<std::vector<T>>
    {
      static constexpr bool value = true;
    };

    template <typename T>
    class abstract_value : public Value
    {
      using Self = abstract_value<T>;

      public:
      abstract_value()
      : m_result(std::make_shared<T>())
      , m_store(m_result.get())
      {
      }

      explicit abstract_value(T* t)
      : m_store(t)
      {
      }

      ~abstract_value() override = default;

      abstract_value& operator=(const abstract_value&) = default;

      abstract_value(const abstract_value& rhs)
      {
        if (rhs.m_result)
        {
          m_result = std::make_shared<T>();
          m_store = m_result.get();
        }
        else
        {
          m_store = rhs.m_store;
        }

        m_default = rhs.m_default;
        m_implicit = rhs.m_implicit;
        m_default_value = rhs.m_default_value;
        m_implicit_value = rhs.m_implicit_value;
      }

      void
      parse(const std::string& text) const override
      {
        parse_value(text, *m_store);
      }

      bool
      is_container() const override
      {
        return type_is_container<T>::value;
      }

      void
      parse() const override
      {
        parse_value(m_default_value, *m_store);
      }

      bool
      has_default() const override
      {
        return m_default;
      }

      bool
      has_implicit() const override
      {
        return m_implicit;
      }

      std::shared_ptr<Value>
      default_value(const std::string& value) override
      {
        m_default = true;
        m_default_value = value;
        return shared_from_this();
      }

      std::shared_ptr<Value>
      implicit_value(const std::string& value) override
      {
        m_implicit = true;
        m_implicit_value = value;
        return shared_from_this();
      }

      std::shared_ptr<Value>
      no_implicit_value() override
      {
        m_implicit = false;
        return shared_from_this();
      }

      std::string
      get_default_value() const override
      {
        return m_default_value;
      }

      std::string
      get_implicit_value() const override
      {
        return m_implicit_value;
      }

      bool
      is_boolean() const override
      {
        return std::is_same<T, bool>::value;
      }

      const T&
      get() const
      {
        if (m_store == nullptr)
        {
          return *m_result;
        }
        return *m_store;
      }

      protected:
      std::shared_ptr<T> m_result{};
      T* m_store{};

      bool m_default = false;
      bool m_implicit = false;

      std::string m_default_value{};
      std::string m_implicit_value{};
    };

    template <typename T>
    class standard_value : public abstract_value<T>
    {
      public:
      using abstract_value<T>::abstract_value;

      CXXOPTS_NODISCARD
      std::shared_ptr<Value>
      clone() const override
      {
        return std::make_shared<standard_value<T>>(*this);
      }
    };

    template <>
    class standard_value<bool> : public abstract_value<bool>
    {
      public:
      ~standard_value() override = default;

      standard_value()
      {
        set_default_and_implicit();
      }

      explicit standard_value(bool* b)
      : abstract_value(b)
      {
        set_default_and_implicit();
      }

      std::shared_ptr<Value>
      clone() const override
      {
        return std::make_shared<standard_value<bool>>(*this);
      }

      private:

      void
      set_default_and_implicit()
      {
        m_default = true;
        m_default_value = "false";
        m_implicit = true;
        m_implicit_value = "true";
      }
    };
  } // namespace values

  template <typename T>
  std::shared_ptr<Value>
  value()
  {
    return std::make_shared<values::standard_value<T>>();
  }

  template <typename T>
  std::shared_ptr<Value>
  value(T& t)
  {
    return std::make_shared<values::standard_value<T>>(&t);
  }

} // namespace cxxopts

#endif //CXXOPTS_VALUES_HPP_
