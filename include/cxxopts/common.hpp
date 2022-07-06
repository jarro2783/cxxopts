#ifndef CXXOPTS_COMMON_HPP_
#define CXXOPTS_COMMON_HPP_

#include <cstdint>

#if __cplusplus >= 201603L
#define CXXOPTS_NODISCARD [[nodiscard]]
#else
#define CXXOPTS_NODISCARD
#endif

#define CXXOPTS__VERSION_MAJOR 3
#define CXXOPTS__VERSION_MINOR 0
#define CXXOPTS__VERSION_PATCH 0

namespace cxxopts
{
  static constexpr struct {
    uint8_t major, minor, patch;
  } version = {
    CXXOPTS__VERSION_MAJOR,
    CXXOPTS__VERSION_MINOR,
    CXXOPTS__VERSION_PATCH
  };
} // namespace cxxopts


#endif //CXXOPTS_COMMON_HPP_
