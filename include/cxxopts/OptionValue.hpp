#ifndef CXXOPTS_OPTIONVALUE_HPP_
#define CXXOPTS_OPTIONVALUE_HPP_

#include "common.hpp"
#include "String.hpp"
#include "exceptions.hpp"
#include "values.hpp"

#include <cstddef>
#include <string>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#if (__GNUC__ < 10 || (__GNUC__ == 10 && __GNUC_MINOR__ < 1)) && __GNUC__ >= 6
#define CXXOPTS_NULL_DEREF_IGNORE
#endif

namespace cxxopts
{
  class OptionDetails
  {
  public:
    OptionDetails
    (
    std::string short_,
    std::string long_,
    String desc,
    std::shared_ptr<const Value> val
    )
    : m_short(std::move(short_))
    , m_long(std::move(long_))
    , m_desc(std::move(desc))
    , m_value(std::move(val))
    , m_count(0)
    {
      m_hash = std::hash<std::string>{}(m_long + m_short);
    }

    OptionDetails(const OptionDetails& rhs)
    : m_desc(rhs.m_desc)
    , m_value(rhs.m_value->clone())
    , m_count(rhs.m_count)
    {
    }

    OptionDetails(OptionDetails&& rhs) = default;

    CXXOPTS_NODISCARD
    const String&
    description() const
    {
      return m_desc;
    }

    CXXOPTS_NODISCARD
    const Value&
    value() const {
      return *m_value;
    }

    CXXOPTS_NODISCARD
    std::shared_ptr<Value>
    make_storage() const
    {
      return m_value->clone();
    }

    CXXOPTS_NODISCARD
    const std::string&
    short_name() const
    {
      return m_short;
    }

    CXXOPTS_NODISCARD
    const std::string&
    long_name() const
    {
      return m_long;
    }

    size_t
    hash() const
    {
      return m_hash;
    }

  private:
    std::string m_short{};
    std::string m_long{};
    String m_desc{};
    std::shared_ptr<const Value> m_value{};
    int m_count;

    size_t m_hash{};
  };

  struct HelpOptionDetails
  {
    std::string s;
    std::string l;
    String desc;
    bool has_default;
    std::string default_value;
    bool has_implicit;
    std::string implicit_value;
    std::string arg_help;
    bool is_container;
    bool is_boolean;
  };

  struct HelpGroupDetails
  {
    std::string name{};
    std::string description{};
    std::vector<HelpOptionDetails> options{};
  };

  class OptionValue
  {
    public:
    void
    parse
    (
      const std::shared_ptr<const OptionDetails>& details,
      const std::string& text
    )
    {
      ensure_value(details);
      ++m_count;
      m_value->parse(text);
      m_long_name = &details->long_name();
    }

    void
    parse_default(const std::shared_ptr<const OptionDetails>& details)
    {
      ensure_value(details);
      m_default = true;
      m_long_name = &details->long_name();
      m_value->parse();
    }

    void
    parse_no_value(const std::shared_ptr<const OptionDetails>& details)
    {
      m_long_name = &details->long_name();
    }

#if defined(CXXOPTS_NULL_DEREF_IGNORE)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnull-dereference"
#endif

    CXXOPTS_NODISCARD
    size_t
    count() const noexcept
    {
      return m_count;
    }

#if defined(CXXOPTS_NULL_DEREF_IGNORE)
#pragma GCC diagnostic pop
#endif

    // TODO: maybe default options should count towards the number of arguments
    CXXOPTS_NODISCARD
    bool
    has_default() const noexcept
    {
      return m_default;
    }

    template <typename T>
    const T&
    as() const
    {
      if (m_value == nullptr) {
          throw_or_mimic<option_has_no_value_exception>(
              m_long_name == nullptr ? "" : *m_long_name);
      }

#ifdef CXXOPTS_NO_RTTI
      return static_cast<const values::standard_value<T>&>(*m_value).get();
#else
      return dynamic_cast<const values::standard_value<T>&>(*m_value).get();
#endif
    }

    private:
    void
    ensure_value(const std::shared_ptr<const OptionDetails>& details)
    {
      if (m_value == nullptr)
      {
        m_value = details->make_storage();
      }
    }


    const std::string* m_long_name = nullptr;
    // Holding this pointer is safe, since OptionValue's only exist in key-value pairs,
    // where the key has the string we point to.
    std::shared_ptr<Value> m_value{};
    size_t m_count = 0;
    bool m_default = false;
  };

  using ParsedHashMap = std::unordered_map<size_t, OptionValue>;
  using NameHashMap = std::unordered_map<std::string, size_t>;

} // namespace cxxopts

#endif //CXXOPTS_OPTIONVALUE_HPP_
