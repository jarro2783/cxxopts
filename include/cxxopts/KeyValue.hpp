#ifndef CXXOPTS_KEYVALUE_HPP_
#define CXXOPTS_KEYVALUE_HPP_

#include "common.hpp"
#include "values.hpp"

#include <string>
#include <utility>

namespace cxxopts {

  class KeyValue
  {
  public:
    KeyValue(std::string key_, std::string value_)
    : m_key(std::move(key_))
    , m_value(std::move(value_))
    {
    }

    CXXOPTS_NODISCARD
    const std::string&
    key() const
    {
      return m_key;
    }

    CXXOPTS_NODISCARD
    const std::string&
    value() const
    {
      return m_value;
    }

    template <typename T>
    T
    as() const
    {
      T result;
      values::parse_value(m_value, result);
      return result;
    }

  private:
    std::string m_key;
    std::string m_value;
  };


} // namespace cxxopts
#endif //CXXOPTS_KEYVALUE_HPP_
