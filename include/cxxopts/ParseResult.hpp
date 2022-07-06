#ifndef CXXOPTS_PARSERESULT_HPP_
#define CXXOPTS_PARSERESULT_HPP_

#include "KeyValue.hpp"
#include "Option.hpp"
#include "OptionValue.hpp"

#include <cstddef>
#include <iterator>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace cxxopts {

  using ParsedHashMap = std::unordered_map<size_t, OptionValue>;
  using NameHashMap = std::unordered_map<std::string, size_t>;

  class ParseResult {
  public:
    class Iterator {
    public:
      using iterator_category = std::forward_iterator_tag;
      using value_type = KeyValue;
      using difference_type = void;
      using pointer = const KeyValue *;
      using reference = const KeyValue &;

      Iterator() = default;

      Iterator(const Iterator &) = default;

      Iterator(const ParseResult *pr, bool end = false)
      : m_pr(pr), m_iter(end ? pr->m_defaults.end() : pr->m_sequential.begin())
      {
      }

      Iterator &operator++()
      {
        ++m_iter;
        if (m_iter == m_pr->m_sequential.end()) {
          m_iter = m_pr->m_defaults.begin();
          return *this;
        }
        return *this;
      }

      Iterator operator++(int)
      {
        Iterator retval = *this;
        ++(*this);
        return retval;
      }

      bool operator==(const Iterator &other) const
      {
        return m_iter == other.m_iter;
      }

      bool operator!=(const Iterator &other) const
      {
        return !(*this == other);
      }

      const KeyValue &operator*()
      {
        return *m_iter;
      }

      const KeyValue *operator->()
      {
        return m_iter.operator->();
      }

    private:
      const ParseResult *m_pr;
      std::vector<KeyValue>::const_iterator m_iter;
    };

    ParseResult() = default;

    ParseResult(const ParseResult &) = default;

    ParseResult(NameHashMap &&keys,
    ParsedHashMap &&values,
    std::vector<KeyValue> sequential,
    std::vector<KeyValue> default_opts,
    std::vector<std::string> &&unmatched_args
    )
    : m_keys(std::move(keys)), m_values(std::move(values)),
      m_sequential(std::move(sequential)), m_defaults(std::move(default_opts)),
      m_unmatched(std::move(unmatched_args))
    {
    }

    ParseResult &operator=(ParseResult &&) = default;

    ParseResult &operator=(const ParseResult &) = default;

    Iterator
    begin() const
    {
      return Iterator(this);
    }

    Iterator
    end() const
    {
      return Iterator(this, true);
    }

    size_t
    count(const std::string &o) const
    {
      auto iter = m_keys.find(o);
      if (iter == m_keys.end()) {
        return 0;
      }

      auto viter = m_values.find(iter->second);

      if (viter == m_values.end()) {
        return 0;
      }

      return viter->second.count();
    }

    const OptionValue &
    operator[](const std::string &option) const
    {
      auto iter = m_keys.find(option);

      if (iter == m_keys.end()) {
        throw_or_mimic<option_not_present_exception>(option);
      }

      auto viter = m_values.find(iter->second);

      if (viter == m_values.end()) {
        throw_or_mimic<option_not_present_exception>(option);
      }

      return viter->second;
    }

    const std::vector<KeyValue> &
    arguments() const
    {
      return m_sequential;
    }

    const std::vector<std::string> &
    unmatched() const
    {
      return m_unmatched;
    }

    const std::vector<KeyValue> &
    defaults() const
    {
      return m_defaults;
    }

    const std::string
    arguments_string() const
    {
      std::string result;
      for (const auto &kv: m_sequential) {
        result += kv.key() + " = " + kv.value() + "\n";
      }
      for (const auto &kv: m_defaults) {
        result += kv.key() + " = " + kv.value() + " " + "(default)" + "\n";
      }
      return result;
    }

  private:
    NameHashMap m_keys{};
    ParsedHashMap m_values{};
    std::vector<KeyValue> m_sequential{};
    std::vector<KeyValue> m_defaults{};
    std::vector<std::string> m_unmatched{};
  };

} // namespace cxxopts

#endif //CXXOPTS_PARSERESULT_HPP_
