#ifndef CXXOPTS_STRING_HPP_
#define CXXOPTS_STRING_HPP_

#include "common.hpp"

#include <cstring>
#include <string>
#include <utility>


//when we ask cxxopts to use Unicode, help strings are processed using ICU,
//which results in the correct lengths being computed for strings when they
//are formatted for the help output
//it is necessary to make sure that <unicode/unistr.h> can be found by the
//compiler, and that icu-uc is linked in to the binary.

#ifdef CXXOPTS_USE_UNICODE
#include <unicode/unistr.h>

namespace cxxopts
{
  using String = icu::UnicodeString;

  inline
  String
  toLocalString(std::string s)
  {
    return icu::UnicodeString::fromUTF8(std::move(s));
  }

#if defined(__GNUC__)
// GNU GCC with -Weffc++ will issue a warning regarding the upcoming class, we want to silence it:
// warning: base class 'class std::enable_shared_from_this<cxxopts::Value>' has accessible non-virtual destructor
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#pragma GCC diagnostic ignored "-Weffc++"
// This will be ignored under other compilers like LLVM clang.
#endif
  class UnicodeStringIterator : public
    std::iterator<std::forward_iterator_tag, int32_t>
  {
    public:

    UnicodeStringIterator(const icu::UnicodeString* string, int32_t pos)
    : s(string)
    , i(pos)
    {
    }

    value_type
    operator*() const
    {
      return s->char32At(i);
    }

    bool
    operator==(const UnicodeStringIterator& rhs) const
    {
      return s == rhs.s && i == rhs.i;
    }

    bool
    operator!=(const UnicodeStringIterator& rhs) const
    {
      return !(*this == rhs);
    }

    UnicodeStringIterator&
    operator++()
    {
      ++i;
      return *this;
    }

    UnicodeStringIterator
    operator+(int32_t v)
    {
      return UnicodeStringIterator(s, i + v);
    }

    private:
    const icu::UnicodeString* s;
    int32_t i;
  };
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

  inline
  String&
  stringAppend(String&s, String a)
  {
    return s.append(std::move(a));
  }

  inline
  String&
  stringAppend(String& s, size_t n, UChar32 c)
  {
    for (size_t i = 0; i != n; ++i)
    {
      s.append(c);
    }

    return s;
  }

  template <typename Iterator>
  String&
  stringAppend(String& s, Iterator begin, Iterator end)
  {
    while (begin != end)
    {
      s.append(*begin);
      ++begin;
    }

    return s;
  }

  inline
  size_t
  stringLength(const String& s)
  {
    return s.length();
  }

  inline
  std::string
  toUTF8String(const String& s)
  {
    std::string result;
    s.toUTF8String(result);

    return result;
  }

  inline
  bool
  empty(const String& s)
  {
    return s.isEmpty();
  }
}

namespace std
{
  inline
  cxxopts::UnicodeStringIterator
  begin(const icu::UnicodeString& s)
  {
    return cxxopts::UnicodeStringIterator(&s, 0);
  }

  inline
  cxxopts::UnicodeStringIterator
  end(const icu::UnicodeString& s)
  {
    return cxxopts::UnicodeStringIterator(&s, s.length());
  }
}

//ifdef CXXOPTS_USE_UNICODE
#else

namespace cxxopts
{
  using String = std::string;

  template <typename T>
  T
  toLocalString(T&& t)
  {
    return std::forward<T>(t);
  }

  inline
  size_t
  stringLength(const String& s)
  {
    return s.length();
  }

  inline
  String&
  stringAppend(String&s, const String& a)
  {
    return s.append(a);
  }

  inline
  String&
  stringAppend(String& s, size_t n, char c)
  {
    return s.append(n, c);
  }

  template <typename Iterator>
  String&
  stringAppend(String& s, Iterator begin, Iterator end)
  {
    return s.append(begin, end);
  }

  template <typename T>
  std::string
  toUTF8String(T&& t)
  {
    return std::forward<T>(t);
  }

  inline
  bool
  empty(const std::string& s)
  {
    return s.empty();
  }
} // namespace cxxopts

//ifdef CXXOPTS_USE_UNICODE
#endif

#endif //CXXOPTS_STRING_HPP_
