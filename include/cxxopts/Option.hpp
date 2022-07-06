#ifndef CXXOPTS_OPTION_HPP_
#define CXXOPTS_OPTION_HPP_

#include "common.hpp"
#include "values.hpp"

#include <memory>
#include <utility>
#include <string>

namespace cxxopts
{

  class OptionDetails;

  struct Option
  {
    Option
    (
      std::string opts,
      std::string desc,
      std::shared_ptr<const Value>  value = ::cxxopts::value<bool>(),
      std::string arg_help = ""
    )
    : opts_(std::move(opts))
    , desc_(std::move(desc))
    , value_(std::move(value))
    , arg_help_(std::move(arg_help))
    {
    }

    std::string opts_;
    std::string desc_;
    std::shared_ptr<const Value> value_;
    std::string arg_help_;
  };


} // namespace cxxopts

#endif //CXXOPTS_OPTION_HPP_
